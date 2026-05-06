// Copyright (C) 2026 Moshe Sulamy

#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <type_traits>

namespace bagel
{
	// bagel is a very small ECS helper.
	// ECS means:
	// - Entity: just an id number.
	// - Component: plain data attached to an entity id.
	// - System: normal code that loops over entities with the components it needs.
	// This file does not know about Pong. Pong decides which components exist and
	// which systems should run.

	/**** Parameters ****/
	// Maximum number of different component types that can be represented in a mask.
	// The Pong demo has exactly six component types.
	constexpr int	MaxComponents = 6;

	// If true, internal arrays can grow when more entities/components are created.
	// If false, the fixed-size StaticBag version is used.
	constexpr bool	DynamicBags = true;
	/** end parameters **/

	// Entity ids are small integers. The id is used as a key into component storage.
	using id_type = int;
	struct ent_type { id_type id; };

	// A mask is a group of bits. Each component type gets one bit.
	// The integer type is chosen small or large enough for MaxComponents.
	using mask_type =
		std::conditional_t<MaxComponents<=8, std::uint_fast8_t,
		std::conditional_t<MaxComponents<=16, std::uint_fast16_t,
		std::conditional_t<MaxComponents<=32, std::uint_fast32_t,
			std::uint_fast64_t>>>;


	// Utility base classes.
	// NoInstance marks classes that are only namespaces for static functions/data.
	// NoCopy prevents accidental copies of objects that own memory.
	struct NoInstance {	NoInstance() = delete; };
	struct NoCopy {
		NoCopy() = default; // default constructor
		NoCopy(const NoCopy&) = delete;
		NoCopy& operator=(const NoCopy&) = delete;
	};

	template <class T, int N>
	// Simple fixed-capacity array with stack-like push/pop.
	// It is easy to understand but cannot grow past N.
	class StaticBag
	{
	public:
		int size() const { return _size; }
		static void ensure(int) {}
		void push(const T& val) { _arr[_size++] = val; }
		T pop() { return _arr[--_size]; }

		T& operator[](int idx) { return _arr[idx]; }
		const T& operator[](int idx) const { return _arr[idx]; }
	private:
		T	_arr[N];
		int _size = 0;
	};
	template <class T, int N>
	// DynamicBag is the growable version of StaticBag.
	// It starts with capacity N and reallocates when more room is needed.
	class DynamicBag : NoCopy
	{
	public:
		int size() const { return _size; }
		void ensure(int new_capacity) {
			if (new_capacity > _capacity) {
				_capacity = std::max(_capacity*2, new_capacity);
				_arr = static_cast<T*>(
					realloc(_arr, sizeof(T)*static_cast<std::size_t>(_capacity)));
			}
		}
		void push(const T& val) {
			if (_size == _capacity) {
				_capacity *= 2;
				_arr = static_cast<T*>(
					realloc(_arr, sizeof(T)*static_cast<std::size_t>(_capacity)));
			}
			_arr[_size] = val;
			++_size;
		}
		T pop() {
			return _arr[--_size];
		}
		~DynamicBag() {
			free(_arr);
		}

		T& operator[](int idx) { return _arr[idx]; }
		const T& operator[](int idx) const { return _arr[idx]; }
	private:
		T*		_arr = static_cast<T*>(malloc(sizeof(T) * static_cast<std::size_t>(N)));
		int		_size = 0;
		int		_capacity = N;
	};

	// Bag is the small array type used everywhere below.
	// Changing DynamicBags switches the whole ECS between fixed and growable storage.
	template <class T, int N>
	using Bag = std::conditional_t<DynamicBags, DynamicBag<T,N>, StaticBag<T,N>>;

	// Each component storage can optionally register a delete function.
	// When an entity is destroyed, World can call the delete function for each
	// component bit the entity has.
	using DeleteFunc = void (*)(ent_type);
	template <class T> struct Register;

	template <class T>
	// SparseStorage stores component T directly at index entity.id.
	// Easy mental model: component for entity 7 lives in _comps[7].
	// Tradeoff: fast lookup, but it can waste space when ids have gaps.
	class SparseStorage final : NoInstance
	{
	public:
		static void add(ent_type ent, const T& val) {
			_comps.ensure(ent.id+1);
			_comps[ent.id] = val;
		}
		static void del(ent_type) {}
		static T& get(ent_type ent) {
			return _comps[ent.id];
		}
	private:
		static inline Bag<T,100> _comps;
		__attribute__((used)) static inline Register<T> _reg{nullptr};
	};
	template <class T>
	// TaggedStorage is for marker components that have no data.
	// Example idea: "IsEnemy" or "InputControlled".
	// The mask bit is enough, so add/del do not store an actual object.
	class TaggedStorage final : NoInstance
	{
	public:
		static void add(ent_type, const T&) {}
		static void del(ent_type) {}
		static T& get(ent_type) = delete;
	private:
		__attribute__((used)) static inline Register<T> _reg{nullptr};
	};
	template <class T>
	// PackedStorage stores only entities that actually have component T.
	// It keeps components tightly packed, which is good for systems that iterate a lot.
	// Extra lookup arrays translate entity id <-> packed component index.
	class PackedStorage final : NoInstance
	{
	public:
		static void add(const ent_type ent, const T& val) {
			_idToComp.ensure(ent.id+1);
			_idToComp[ent.id] = _comps.size();
			_comps.push(val);
			_compToId.push(ent.id);
		}
		static void del(const ent_type ent) {
			int idx = _idToComp[ent.id];
			const id_type last = _compToId.pop();

			// Deleting from the middle would leave a hole.
			// Instead, move the last component into the deleted slot and update maps.
			_comps[idx] = _comps.pop();
			_compToId[idx] = last;
			_idToComp[last] = idx;
		}
		static T& get(const ent_type ent) {
			return _comps[_idToComp[ent.id]];
		}
	private:
		static inline Bag<T,100> _comps;
		static inline Bag<int,100> _idToComp;
		static inline Bag<id_type,100> _compToId;
		__attribute__((used)) static inline Register<T> _reg{del};
	};
	template <class T>
	// StackStorage reuses empty component slots after deletion.
	// It is useful when components are created/destroyed often and you do not
	// want every delete to move the last component like PackedStorage does.
	class StackStorage final : NoInstance
	{
	public:
		static void add(const ent_type ent, const T& val) {
			_idToComp.ensure(ent.id+1);
			if (_freeIdx.size() > 0) {
				const int idx = _freeIdx.pop();
				_idToComp[ent.id] = idx;
				_comps[idx] = val;
			}
			else {
				_idToComp.ensure(ent.id+1);
				_idToComp[ent.id] = _comps.size();
				_comps.push(val);
			}
			//TODO: remember empty/full cells
		}
		static void del(const ent_type ent) {
			_freeIdx.push(_idToComp[ent.id]);
		}
		static T& get(const ent_type ent) {
			return _comps[_idToComp[ent.id]];
		}
	private:
		static inline Bag<T,100> _comps;
		static inline Bag<int,100> _idToComp;
		static inline Bag<id_type,100> _freeIdx;
		__attribute__((used)) static inline Register<T> _reg{del};
	};

	template <class T>
	// Default storage choice for a component type.
	// Game code can specialize bagel::Storage<MyComponent> to choose a different
	// storage type, as Pong does for Keys and Intent.
	struct Storage final : NoInstance {
		using type = SparseStorage<T>;
	};

	// Mask answers questions like:
	// "Does this entity have Transform and Drawable?"
	// Each component type has one bit. An entity mask contains all bits for the
	// components attached to that entity.
	class Mask final
	{
	public:
		using bit_type = mask_type;

		// Convert a component index into its bit value.
		// Index 0 -> 000001, index 1 -> 000010, and so on.
		static constexpr bit_type bit(const int idx) {
			return static_cast<bit_type>(static_cast<mask_type>(1) << idx);
		}

		void set(const bit_type b) { _mask |= b; }

		void clear(const bit_type b) { _mask &= ~b; }
		void clear() { _mask = 0; }

		// test(bit) checks one component bit.
		bool test(const bit_type b) const { return _mask & b; }

		// test(mask) checks whether all bits from another mask are present.
		// This is why systems can build a mask and ask every entity "do you match?"
		bool test(const Mask m) const { return (_mask & m._mask) == m._mask; }

		// Count trailing zeroes finds the index of the lowest set bit.
		// World uses this while deleting an entity to visit every component it has.
		int ctz() const { return _mask ? __builtin_ctz(_mask) : -1; }
	private:
		mask_type	_mask{0};
	};

	static inline int compCounter = -1;
	template <class>
	// Component<T> gives every component type a unique index and bit.
	// The first time Component<Transform> is used it gets the next free bit.
	struct Component final : NoInstance
	{
		static inline const int				Index = ++compCounter;
		static inline const Mask::bit_type	Bit = Mask::bit(Index);
	};

	// World is the central static database:
	// - creates/reuses entity ids
	// - stores the mask for each entity
	// - routes add/get/delete operations to the selected Storage<T>::type
	class World final : NoInstance
	{
	public:
		static ent_type createEntity() {
			// Reuse a previously destroyed id if possible.
			if (_ids.size() > 0)
				return {_ids.pop()};

			// Otherwise create a new id and give it an empty component mask.
			_masks.push(Mask{});
			return {++_maxId};
		}
		static void deleteEntity(ent_type ent) {
			// Copy the mask, then walk through all component bits the entity has.
			Mask m = _masks[ent.id];
			int ctz;
			while ((ctz = m.ctz()) >= 0) {
				if (_deleters[ctz] != nullptr)
					_deleters[ctz](ent);
				m.clear(Mask::bit(ctz));
			}
			_masks[ent.id].clear();
			_ids.push(ent.id);
		}

		// Read the mask for one entity. Systems use this to test component sets.
		static const Mask& mask(ent_type e) {
			return _masks[e.id];
		}

		template <class T>
		static T& getComponent(ent_type e) {
			return Storage<T>::type::get(e);
		}

		template <class T>
		static void addComponent(ent_type ent, const T& comp) {
			// Adding a component has two parts:
			// 1. set the component bit in the entity mask
			// 2. store the actual component data in that component's storage
			_masks[ent.id].set(Component<T>::Bit);
			Storage<T>::type::add(ent,comp);
		}

		template <class T>
		static void delComponent(ent_type ent, const T& comp) {
			_masks[ent.id].clear(Component<T>::Bit);
			Storage<T>::type::del(ent,comp);
		}

		template <class T>
		static void registerDeleter(DeleteFunc func) {
			// Store the delete function at the component type's index.
			while (_deleters.size() < Component<T>::Index+1)
				_deleters.push(nullptr);
			_deleters[Component<T>::Index] = func;
		}

		static id_type maxId() { return _maxId; }
	private:
		static inline Bag<Mask,100>		_masks;
		static inline Bag<id_type,100>	_ids;
		static inline Bag<DeleteFunc,10> _deleters;
		static inline id_type _maxId = -1;
	};

	// Register<T> runs during static initialization for each storage class.
	// It tells World how to delete component T from storage when an entity dies.
	template <class T> struct Register
	{
		explicit Register(const DeleteFunc func) {
			World::registerDeleter<T>(func);
		}
	};

	// Convenience builder for system masks.
	// Example:
	// MaskBuilder().set<Transform>().set<Drawable>().build()
	// means "entities that have both Transform and Drawable".
	class MaskBuilder
	{
	public:
		template <class T>
		MaskBuilder& set() {
			m.set(Component<T>::Bit);
			return *this;
		}
		Mask build() const { return m; }
	private:
		Mask m;
	};

	// Entity is a small wrapper around ent_type.
	// It makes code read naturally: Entity::create().addAll(...)
	// The actual data still lives in World and the component storages.
	class Entity
	{
	public:
		Entity(ent_type ent) : _ent(ent) {}
		ent_type entity() const { return _ent; }

		static Entity create() { return World::createEntity(); }
		void destroy() const { World::deleteEntity(_ent); }

		const Mask& mask() const { return World::mask(_ent); }

		// Component access helpers for this entity id.
		template <class T> T& get() const {
			return World::getComponent<T>(_ent);
		}
		template <class T> void add(const T& val) const {
			World::addComponent<T>(_ent,val);
		}
		template <class T> void del() const {
			World::delComponent<T>(_ent);
		}

		// Add or remove several components in one call.
		template <class T, class ...Ts> void addAll(const T& val, const Ts&... vals) const {
			add(val);
			if constexpr (sizeof...(Ts)>0)
				addAll(vals...);
		}
		template <class T, class ...Ts> void delAll() const {
			del<T>();
			if constexpr (sizeof...(Ts)>0)
				del<Ts...>();
		}

		template <class T> bool has() const { return mask().test(Component<T>::Bit); }
		bool test(const Mask& m) const { return mask().test(m); }

		// Simple iteration over all possible entity ids.
		// Systems use this pattern:
		// for (Entity e = Entity::first(); !e.eof(); e.next()) { ... }
		static Entity first() { return Entity{{0}}; }
		bool eof() const { return _ent.id > World::maxId(); }
		void next() { ++_ent.id; }
	private:
		ent_type _ent;
	};
}
