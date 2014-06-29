#ifndef __EZ_JSON__
#define __EZ_JSON__

#include <string>
#include <vector>
#include <memory>
#include <type_traits>

namespace Ez
{

class Node;
class FastAllocator;

class JSON
{
private:

	std::shared_ptr<FastAllocator> allocator;
	Node *node;

public:

	explicit JSON(const char *content);

	size_t size() const;
	std::vector<std::string> keys() const;

	double asDouble() const;
	bool asBool() const;
	std::string asString() const;
	std::string serialize() const;

	template <typename T>
	typename std::enable_if<std::is_integral<T>::value, JSON>::type
		operator[](T idx) const
	{
		return at(idx);
	}

	template <typename T>
	typename std::enable_if<std::is_same<T, const char*>::value, JSON>::type
		operator[](T k) const
	{
		return key(k);
	}

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
	
	JSON(Node* nd, std::shared_ptr<FastAllocator> alc);
	Node* parse(const char *content, FastAllocator& alc) const;

	JSON at(size_t idx) const;
	JSON key(const char *key) const;
	void setAt(size_t idx, const char*);
	void setKey(const char *k, const char*);
	void removeAt(size_t idx);
	void removeKey(const char *k);
};

} // namespace Ez

#endif