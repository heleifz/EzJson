#ifndef __EZ_JSON__
#define __EZ_JSON__

#include <string>
#include <memory>
#include <vector>
#include <type_traits>

struct Node;
class ChunkAllocator;

class EzJSON {
private:

	std::shared_ptr<ChunkAllocator> allocator;
	Node *node;

public:

	explicit EzJSON(const char *content);

	size_t size() const;
	std::vector<std::string> keys() const;

	double asDouble() const;
	bool asBool() const;
	std::string asString() const;
	std::string serialize() const;

	template <typename T>
	typename std::enable_if<std::is_integral<T>::value, EzJSON>::type
	operator[](T idx) const
	{
		return at(idx);
	}

	template <typename T>
	typename std::enable_if<std::is_same<T, const char*>::value, EzJSON>::type
	operator[](T k) const
	{
		return key(k);
	}

	// Modify AST
	void append(const char *other);

	template <typename T>
	typename std::enable_if<std::is_integral<T>::value, void>::type
	set(T idx, const char *content)
	{
		setAt(idx, content);
	}

	template <typename T>
	typename std::enable_if<std::is_same<T, const char*>::value, void>::type
	set(T k, const char *content)
	{
		setKey(k, content);
	}

	template <typename T>
	typename std::enable_if<std::is_integral<T>::value, void>::type
	remove(T idx)
	{
		removeAt(idx);
	}

	template <typename T>
	typename std::enable_if<std::is_same<T, const char*>::value, void>::type
	remove(T k)
	{
		removeKey(k);
	}

private:
	EzJSON at(size_t idx) const;
	EzJSON key(const char *key) const;
	void setAt(size_t idx, const char*);
	void setKey(const char *k, const char*);
	void removeAt(size_t idx);
	void removeKey(const char *k);

	Node* parse(const char *content, ChunkAllocator& alc) const;

	EzJSON(Node* nd, std::shared_ptr<ChunkAllocator> alc);
};

#endif