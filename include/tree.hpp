#ifndef COMMON_TREE_H
#define COMMON_TREE_H

#include <vector>

template<typename T>
class heirarchy {
	std::vector<heirarchy<T>* > children;
	std::vector<T> list;
public:
	void setNodeProcessingCB(void (*cb)(heirarchy<T>&, int));
};


#endif /* EOF */
