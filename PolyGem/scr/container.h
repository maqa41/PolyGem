#pragma once
#include "benchmark.h"
#include <bit>

namespace container {
	static uint64_t universalOne = 1;

	template<typename T_obj, typename T_func>
	class Node {
	public:
		T_obj data;
		Node* link = NULL;

		~Node() {
			Log(data);
			if constexpr (std::is_pointer_v<T_obj>) {
				delete data;
			}
			Log("Node deleted!");
		}

		static void insertNode(Node** head, T_obj data) {
			if (head == NULL) {
				Log("Error: Current Node is passed as NULL!");
				return;
			}
			else {
				Node* next = new Node;
				next->data = data;
				next->link = (*head);
				(*head) = next;
			}
		}

		static void deleteNode(Node** prev, Node** curr) {
			if (prev == NULL) {
				Log("Prev Node is not passed!");
				return;
			}
			else if (*prev == *curr) {
				Log("Warning: Deletion denied! Prev Node is same as the Curr Node!");
				return;
			}
			else {
				(*prev)->link = (*curr)->link;
				delete (*curr);
			}
		}

		static void traverse(Node* head, T_func funcObj, void(*func)(T_func, T_obj&, bool&, bool&) = [](T_func funcParam, T_obj& value, bool& specCondition, bool& specCase) {Log(value); }, bool(*check)(T_obj) = [](T_obj value) {return false; }, bool specialCase = false) {
			Node* ptr = head;
			Node* prev = head;
			bool specialCondition = false;
			while (ptr != NULL && (!specialCondition)) {
				func(funcObj, ptr->data, specialCondition, specialCase);
				if (check(ptr->data)) {
					Node* ptrNew = ptr->link;
					deleteNode(&prev, &ptr);
					ptr = ptrNew;
				}
				else {
					prev = ptr;
					ptr = ptr->link;
				}
			}
		}

		static void destroyNodes(Node* head) {
			Node* ptr = head;
			Node* prev = head;
			while (ptr != NULL) {
				Node* ptrNew = ptr->link;
				deleteNode(&prev, &ptr);
				ptr = ptrNew;
			}
			delete prev;
		}
	};

	template<typename List>
	class ListIterator {
	public:
		using ObjectType = typename List::ObjectType;
		using PointerType = ObjectType*;
		using ReferenceType = ObjectType&;
		const PointerType end_ptr;

	private:
		PointerType obj_ptr;
		PointerType begin_ptr;
		uint64_t* m_EmptySlots;
		size_t m_EmptySlotCapacity;
		
		bool CheckEmptySlotIndex(size_t index) {
			size_t emptySlotBlock = index >> 6;
			size_t emptySlotIndex = index & 0x3F;
			return (m_EmptySlots[emptySlotBlock] & (universalOne << emptySlotIndex)) == 0;
		}

		uint64_t GetFilledSlotIndex(size_t index) {
			size_t emptySlotBlock = index >> 6;
			size_t slotIndex = index & 0x3F;
			uint64_t filledSlotIndex = std::countr_zero((m_EmptySlots[emptySlotBlock] >> slotIndex));
			return filledSlotIndex;
		}

	public:

		ListIterator(PointerType ptr, const PointerType end, uint64_t* _emptySlots, size_t capacity) : obj_ptr(ptr), end_ptr(end), m_EmptySlots(_emptySlots), m_EmptySlotCapacity(capacity) {
			begin_ptr = obj_ptr;
			size_t emptySlotBlock = 0;
			size_t slotIndex = 0;
			uint64_t filledSlotIndex = std::countr_zero((m_EmptySlots[emptySlotBlock] >> slotIndex));

			while (emptySlotBlock < m_EmptySlotCapacity && filledSlotIndex == 64) {
				emptySlotBlock++;
				filledSlotIndex = std::countr_zero(m_EmptySlots[emptySlotBlock]);
			}
			obj_ptr += (emptySlotBlock << 6) + filledSlotIndex;
		}

		ListIterator(const ListIterator& other) 
			: obj_ptr(other.obj_ptr), begin_ptr(other.begin_ptr), end_ptr(other.end_ptr),
			m_EmptySlots(other.m_EmptySlots), m_EmptySlotCapacity(other.m_EmptySlotCapacity) { }

		ListIterator& operator=(const ListIterator& other) {
			if (this != *other) {
				obj_ptr = other.obj_ptr;
				begin_ptr = other.begin_ptr;
				end_ptr = other.end_ptr;
				m_EmptySlots = other.m_EmptySlots;
				m_EmptySlotCapacity = other.m_EmptySlotCapacity;
			}
			return *this;
		}

		const PointerType GetBegin() {
			return begin_ptr;
		}

		size_t GetIndex() {
			return (size_t)(obj_ptr - begin_ptr);
		}

		ListIterator& operator++() {
			obj_ptr++;
			size_t index = (size_t)(obj_ptr - begin_ptr);
			size_t emptySlotBlock = index >> 6;
			size_t slotIndex = index & 0x3F;
			uint64_t filledSlotIndex = std::countr_zero((m_EmptySlots[emptySlotBlock] >> slotIndex));

			while (emptySlotBlock < m_EmptySlotCapacity && filledSlotIndex == 64) {
				emptySlotBlock++;
				obj_ptr = begin_ptr + (emptySlotBlock << 6);
				filledSlotIndex = std::countr_zero(m_EmptySlots[emptySlotBlock]);
			}
			obj_ptr += filledSlotIndex;

			return *this;
		}

		ListIterator operator++(int) {
			ListIterator iterator = *this;
			++(*this);
			return iterator;
		}

		ListIterator& operator+(int inc) {
			ListIterator newIter = *this;
			newIter.obj_ptr += inc;
			size_t index = (size_t)(newIter.obj_ptr - newIter.begin_ptr);
			size_t emptySlotBlock = index >> 6;
			size_t slotIndex = index & 0x3F;
			uint64_t filledSlotIndex = std::countr_zero((newIter.m_EmptySlots[emptySlotBlock] >> slotIndex));

			while (emptySlotBlock < newIter.m_EmptySlotCapacity && filledSlotIndex == 64) {
				emptySlotBlock++;
				newIter.obj_ptr = newIter.begin_ptr + (emptySlotBlock << 6);
				filledSlotIndex = std::countr_zero(newIter.m_EmptySlots[emptySlotBlock]);
			}
			newIter.obj_ptr += filledSlotIndex;

			return newIter;
		}

		ReferenceType operator[](const size_t index) {
			return *(obj_ptr + index);
		}

		PointerType operator->() {
			return obj_ptr;
		}

		ReferenceType operator*() {
			return *obj_ptr;
		}

		bool operator==(const PointerType& other) const {
			return obj_ptr == other;
		}

		bool operator==(const ListIterator& other) const {
			return obj_ptr == other.obj_ptr;
		}

		bool operator!=(const PointerType& other) const {
			return obj_ptr != other;
		}

		bool operator!=(const ListIterator& other) const {
			return obj_ptr != other.obj_ptr;
		}

		bool operator<(const PointerType& other) const {
			return obj_ptr < other;
		}

		bool operator<(const ListIterator& other) const {
			return obj_ptr < other.obj_ptr;
		}

		bool operator>(const PointerType& other) const {
			return obj_ptr > other;
		}

		bool operator>(const ListIterator& other) const {
			return obj_ptr > other.obj_ptr;
		}
	};

	template<typename T_obj>
	class List {
	public:
		using ObjectType = T_obj;
		using Iterator = ListIterator<List<T_obj>>;

	private:
		T_obj* m_Objects;
		size_t m_ObjectCount = 0;
		size_t m_Capacity;
		size_t m_EmptySlotCapacity;
		uint64_t* m_EmptySlots;

		void ReallocateMemory() {
			size_t newCapacity = (size_t)((float)m_Capacity * 1.5f);
			if ((m_EmptySlotCapacity << 6) < newCapacity) {
				size_t newEmptySlotCapacity = (newCapacity >> 6) + 1;
				uint64_t* newEmptySlots = new uint64_t[newEmptySlotCapacity];
				for (uint64_t slot = 0; slot < m_EmptySlotCapacity; slot++) {
					newEmptySlots[slot] = m_EmptySlots[slot];
				}
				for (size_t slot = m_EmptySlotCapacity; slot < newEmptySlotCapacity; slot++) {
					newEmptySlots[slot] = 0;
				}
				delete[] m_EmptySlots;
				m_EmptySlots = newEmptySlots;
				m_EmptySlotCapacity = newEmptySlotCapacity;
			}
			T_obj* newObjects = (T_obj*)::operator new(newCapacity * sizeof(T_obj));
			for (size_t index = 0; index < m_Capacity; index++) {
				new(&newObjects[index]) T_obj(std::move(m_Objects[index]));
			}
			for (size_t index = 0; index < m_Capacity; index++) {
				m_Objects[index].~T_obj();
			}
			::operator delete(m_Objects, m_Capacity * sizeof(T_obj));
			m_Objects = newObjects;
			m_Capacity = newCapacity;
		}

		size_t GetEmptySlotIndex() {
			size_t emptySlotBlock = 0;
			uint64_t emptySlotIndex = std::countr_zero(~(m_EmptySlots[emptySlotBlock]));
			while (emptySlotBlock < m_EmptySlotCapacity && emptySlotIndex == 64) {
				emptySlotBlock++;
				emptySlotIndex = std::countr_zero(~(m_EmptySlots[emptySlotBlock]));
			}
			m_EmptySlots[emptySlotBlock] ^= universalOne << emptySlotIndex;
			return (emptySlotBlock << 6) + emptySlotIndex;
		}

		bool CheckEmptySlotIndex(size_t index) const {
			size_t emptySlotBlock = index >> 6;
			size_t emptySlotIndex = index & 0x3F;
			return (m_EmptySlots[emptySlotBlock] & (universalOne << emptySlotIndex)) == 0;
		}

		void SetEmptySlotIndex(size_t index) {
			size_t emptySlotBlock = index >> 6;
			size_t emptySlotIndex = index & 0x3F;
			m_EmptySlots[emptySlotBlock] ^= universalOne << emptySlotIndex;
		}

	public:

		List(size_t MS = 2) {
			m_Capacity = MS;
			m_Objects = (T_obj*)::operator new(m_Capacity * sizeof(T_obj));
			m_EmptySlotCapacity = 1 + (m_Capacity >> 6);
			m_EmptySlots = new uint64_t[m_EmptySlotCapacity];
			for (size_t i = 0; i < m_EmptySlotCapacity; i++) {
				m_EmptySlots[i] = 0;
			}
		}

		List(std::initializer_list<T_obj> objs) {
			m_Capacity = (objs.size() > 2) ? objs.size() : 2;
			m_Objects = (T_obj*)::operator new(m_Capacity * sizeof(T_obj));
			m_EmptySlotCapacity = 1 + (m_Capacity >> 6);
			m_EmptySlots = new uint64_t[m_EmptySlotCapacity];
			for (size_t i = 0; i < m_EmptySlotCapacity; i++) {
				m_EmptySlots[i] = 0;
			}
			for (auto obj : objs) {
				Append(obj);
			}
		}

		List(const List& other) : m_Capacity(other.m_Capacity), m_EmptySlotCapacity(other.m_EmptySlotCapacity), m_ObjectCount(other.m_ObjectCount) {
			m_EmptySlots = new uint64_t[m_EmptySlotCapacity];
			std::memcpy(m_EmptySlots, other.m_EmptySlots, m_EmptySlotCapacity * sizeof(uint64_t));
			m_Objects = (T_obj*)::operator new(m_Capacity * sizeof(T_obj));
			for (size_t index = 0; index < m_Capacity; index++) {
				if (!other.CheckEmptySlotIndex(index)) {
					new(&m_Objects[index]) T_obj(other.m_Objects[index]);
				}
			}
			Log("List Copied!\n");
		}

		List& operator=(const List& other) {
			if (this != &other) {
				m_Capacity = other.m_Capacity;
				m_EmptySlotCapacity = other.m_EmptySlotCapacity;
				m_ObjectCount = other.m_ObjectCount;
				m_EmptySlots = new uint64_t[m_EmptySlotCapacity];
				std::memcpy(m_EmptySlots, other.m_EmptySlots, m_EmptySlotCapacity * sizeof(uint64_t));
				m_Objects = (T_obj*)::operator new(m_Capacity * sizeof(T_obj));
				for (size_t index = 0; index < m_Capacity; index++) {
					if (!other.CheckEmptySlotIndex(index)) {
						new(&m_Objects[index]) T_obj(other.m_Objects[index]);
					}
				}
			}
			Log("List Copied!\n");
			return *this;
		}

		List(List&& other) noexcept : m_Capacity(other.m_Capacity), m_EmptySlotCapacity(other.m_EmptySlotCapacity), m_ObjectCount(other.m_ObjectCount) {
			m_Objects = other.m_Objects;
			other.m_Objects = nullptr;
			m_EmptySlots = other.m_EmptySlots;
			other.m_EmptySlots = nullptr;
			other.m_Capacity = 0;
			other.m_EmptySlotCapacity = 0;
			Log("List Moved!\n");
		}

		List& operator=(List&& other) noexcept {
			if (this != &other) {
				m_Capacity = other.m_Capacity;
				other.m_Capacity = 0;
				m_EmptySlotCapacity = other.m_EmptySlotCapacity;
				other.m_EmptySlotCapacity = 0;
				m_ObjectCount = other.m_ObjectCount;
				m_Objects = other.m_Objects;
				other.m_Objects = nullptr;
				m_EmptySlots = other.m_EmptySlots;
				other.m_EmptySlots = nullptr;
				Log("List Moved!\n");
			}
			return *this;
		}

		~List() {
			Clear();
			::operator delete(m_Objects, m_Capacity * sizeof(T_obj));
			delete[] m_EmptySlots;
			//Log("List destroyed!");
		}

		T_obj& operator[](const size_t index) {
			if (index < m_Capacity && !CheckEmptySlotIndex(index)) {
				return m_Objects[index];
			}
			Log("Error! Index is out of range!", true);
			Log(this);
			throw std::exception();
		}

		void Append(T_obj& object) {
			if (m_ObjectCount == m_Capacity && m_ObjectCount != 0) {
				ReallocateMemory();
			}
			size_t trueEmptySlotIndex = GetEmptySlotIndex();
			new(&m_Objects[trueEmptySlotIndex]) T_obj(object);
			m_ObjectCount += 1;
		}

		void Append(T_obj&& object) {
			if (m_ObjectCount == m_Capacity && m_ObjectCount != 0) {
				ReallocateMemory();
			}
			size_t trueEmptySlotIndex = GetEmptySlotIndex();
			new(&m_Objects[trueEmptySlotIndex]) T_obj(std::move(object));
			m_ObjectCount += 1;
		}

		template<typename...Args>
		void EmplaceBack(Args&&... args) {
			if (m_ObjectCount == m_Capacity && m_ObjectCount != 0) {
				ReallocateMemory();
			}
			size_t trueEmptySlotIndex = GetEmptySlotIndex();
			new(&m_Objects[trueEmptySlotIndex]) T_obj(std::forward<Args>(args)...);
			m_ObjectCount += 1;
		}

		void Remove(const size_t index) {
			if (index < m_Capacity && !CheckEmptySlotIndex(index)) {
				if constexpr (std::is_pointer_v<T_obj>) {
					delete m_Objects[index];
					m_Objects[index] = nullptr;
				}
				else {
					m_Objects[index].~T_obj();
				}
				SetEmptySlotIndex(index);
				m_ObjectCount -= 1;
				return;
			}
			Log("Warning! Index outsite of the accessible memory! ID:", true);
			Log(this);
		}

		void Remove(Iterator item) {
			size_t index = item.GetIndex();
			if constexpr (std::is_pointer_v<T_obj>) {
				delete m_Objects[index];
				m_Objects[index] = nullptr;
			}
			else {
				m_Objects[index].~T_obj();
			}
			SetEmptySlotIndex(index);
			m_ObjectCount -= 1;
			return;
		}

		size_t GetEmptySlot() {
			size_t emptySlotBlock = 0;
			uint64_t emptySlotIndex = std::countr_zero(~(m_EmptySlots[emptySlotBlock]));
			while (emptySlotBlock < m_EmptySlotCapacity && emptySlotIndex == 64) {
				emptySlotBlock++;
				emptySlotIndex = std::countr_zero(~(m_EmptySlots[emptySlotBlock]));
			}
			return (emptySlotBlock << 6) + emptySlotIndex;
		}

		void Clear() {
			for (size_t index = 0; index < m_Capacity; index++) {
				if (!CheckEmptySlotIndex(index)) {
					if constexpr (std::is_pointer_v<T_obj>) {
						delete m_Objects[index];
						m_Objects[index] = nullptr;
					}
					else {
						m_Objects[index].~T_obj();
					}
				}
			}
			for (size_t i = 0; i < m_EmptySlotCapacity; i++) {
				m_EmptySlots[i] = 0;
			}
			m_ObjectCount = 0;
		}

		const size_t GetSize() {
			return m_ObjectCount;
		}

		const size_t GetCapacity() {
			return m_Capacity;
		}

		Iterator Begin() {
			return Iterator(m_Objects, End(), m_EmptySlots, m_EmptySlotCapacity);
		}

		Iterator::PointerType End() {
			return (m_Objects + m_Capacity);
		}
	};
}