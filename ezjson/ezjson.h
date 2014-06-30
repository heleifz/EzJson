#ifndef __EZ_JSON__
#define __EZ_JSON__

#include <string>
#include <vector>
#include <memory>
#include <type_traits>

namespace Ez
{

/**
 * @brief Internal AST node class
 * 
 */
class Node;

/**
 * @brief Internal allocator class
 * 
 */
class FastAllocator;

/**
 * @brief Wrapper class for JSON AST node
 * 
 */
class JSON
{
private:

	// every AST has only one allocator associated with it
	// when every node in that tree is destroyed,
	// the allocator free its memory pool
	std::shared_ptr<FastAllocator> allocator;

	// JSON object is a thin wrapper for an AST node
	Node *node;

public:

	/**
	 * @brief Parsing input string and construct a JSON object
	 * 
	 * @param content JSON string
	 */
	explicit JSON(const char *content);

	/**
	 * @brief Get the size of the node's children (must be array or object)
	 * @return Size of current node
	 */
	size_t size() const;

	/**
	 * @brief Get keys of the node's children
	 * @return keys of current node's children
	 */
	std::vector<std::string> keys() const;

	/**
	 * @brief Convert the node to double
	 * @return double precision value
	 */
	double asDouble() const;

	/**
	 * @brief Convert the node to boolean 
	 * @return boolean value
	 */
	bool asBool() const;

	/**
	 * @brief Convert the node to STL string
	 * @return string
	 */
	std::string asString() const;

	/**
	 * @brief Return a prettified version of the node's subtree
	 * @return prettified JSON string
	 */
	std::string serialize() const;

	/**
	 * @brief Access array node's child
	 * @details Use type function to shut the compiler up
	 * 
	 * @param  idx   Index of the child
	 * @return Child node
	 */
	template <typename T>
	typename std::enable_if<std::is_integral<T>::value, JSON>::type
		operator[](T idx) const
	{
		return at(idx);
	}

	/**
	 * @brief Access object node's child
	 * @details Use type function to shut the compiler up
	 * 
	 * @param  k     key of the child
	 * @return Child node
	 */
	template <typename T>
	typename std::enable_if<std::is_same<T, const char*>::value, JSON>::type
		operator[](T k) const
	{
		return key(k);
	}

	/**
	 * @brief Append new node to current array node
	 * 
	 * @param other JSON string of the new node
	 */
	void append(const char *other);

	/**
	 * @brief Set the value of the child node
	 * @details Use type function to shut the compiler up
	 * 
	 * @param idx Index of the child
	 * @param content JSON string
	 * 
	 */
	template <typename T>
	typename std::enable_if<std::is_integral<T>::value, void>::type
		set(T idx, const char *content)
	{
		setAt(idx, content);
	}

	/**
	 * @brief Set the value of the child node
	 * @details Use type function to shut the compiler up
	 * 
	 * @param k Key of the child
	 * @param content JSON string
	 * 
	 */
	template <typename T>
	typename std::enable_if<std::is_same<T, const char*>::value, void>::type
		set(T k, const char *content)
	{
		setKey(k, content);
	}

	/**
	 * @brief Remove the child node
	 * @details Use type function to shut the compiler up
	 * 
	 * @param  idx Index of the child
	 */
	template <typename T>
	typename std::enable_if<std::is_integral<T>::value, void>::type
		remove(T idx)
	{
		removeAt(idx);
	}

	/**
	 * @brief Remove the child node
	 * @details Use type function to shut the compiler up
	 * 
	 * @param  k Key of the child
	 */
	template <typename T>
	typename std::enable_if<std::is_same<T, const char*>::value, void>::type
		remove(T k)
	{
		removeKey(k);
	}

private:
	
	// used in chaining the indexing operation
	JSON(Node* nd, std::shared_ptr<FastAllocator> alc);

	// construct a AST node from JSON string
	Node* parse(const char *content, FastAllocator& alc) const;

	// implementations of set, remove, operator[]
	JSON at(size_t idx) const;
	JSON key(const char *key) const;
	void setAt(size_t idx, const char*);
	void setKey(const char *k, const char*);
	void removeAt(size_t idx);
	void removeKey(const char *k);
};

} // namespace Ez

#endif