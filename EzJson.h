#ifndef __EZ_JSON__
#define __EZ_JSON__

#include <string>
#include <memory>
#include <vector>

struct Node;
class ChunkAllocator;

class EzJSON {
public:

	explicit EzJSON(const char *content);

	// fluent interface
	size_t size() const;
	std::vector<std::string> fields() const;
	double asDouble() const;
	bool asBool() const;
	std::string asString() const;

	EzJSON at(size_t idx);
	EzJSON field(const char *key);

private:

	EzJSON(Node* nd, std::shared_ptr<ChunkAllocator> alc);
	std::shared_ptr<ChunkAllocator> allocator;
	Node *node;
};

#endif