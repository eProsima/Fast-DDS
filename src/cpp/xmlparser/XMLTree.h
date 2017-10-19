#ifndef _XML_TREE_
#define _XML_TREE_

#include <map>
#include <memory>
#include <vector>

namespace eprosima {
namespace fastrtps {
namespace xmlparser {

enum class NodeType
{
    PROFILES,
    PARTICIPANT,
    PUBLISHER,
    SUBSCRIBER,
    RTPS,
    QOS_PROFILE,
    APPLICATION,
    TYPE,
    TOPIC,
    DATA_WRITER,
    DATA_READER,
    ROOT
};

class BaseNode
{
  public:
    BaseNode(NodeType type) : data_type_(type), parent_(nullptr){};
    virtual ~BaseNode() = default;

    NodeType getType() const
    {
        return data_type_;
    }

    void addChild(std::unique_ptr<BaseNode> child)
    {
        child->setParent(this);
        children.push_back(std::move(child));
    }

    void removeChild(const size_t& indx)
    {
        children.erase(children.begin() + indx);
    }

    BaseNode* getChild(const size_t& indx) const
    {
        return children[indx].get();
    }

    BaseNode* getParent() const
    {
        return parent_;
    }

    void setParent(BaseNode* parent)
    {
        parent_ = parent;
    }

    int getNumChildren() const
    {
        return children.size();
    }

  private:
    NodeType data_type_;
    BaseNode* parent_;
    std::vector<std::unique_ptr<BaseNode>> children;
};

template <class T>
class DataNode : public BaseNode
{
  public:
    DataNode(NodeType type);
    DataNode(NodeType type, std::unique_ptr<T> data);
    virtual ~DataNode();

    T* getData() const;
    void setData(std::unique_ptr<T> data);

    void addAttribute(const std::string& name, const std::string& value);
    const std::map<std::string, std::string>& getAttributes();

  private:
    std::map<std::string, std::string> attributes_;
    std::unique_ptr<T> data_;
};

template <class T>
DataNode<T>::DataNode(NodeType type) : BaseNode(type), attributes_(), data_(nullptr)
{
}

template <class T>
DataNode<T>::DataNode(NodeType type, std::unique_ptr<T> data)
    : BaseNode(type), attributes_(), data_(std::move(data))
{
}

template <class T>
DataNode<T>::~DataNode()
{
}

template <class T>
T* DataNode<T>::getData() const
{
    return data_.get();
}

template <class T>
void DataNode<T>::setData(std::unique_ptr<T> data)
{
    data_ = data;
}

template <class T>
void DataNode<T>::addAttribute(const std::string& name, const std::string& value)
{
    attributes_[name] = value;
}

template <class T>
const std::map<std::string, std::string>& DataNode<T>::getAttributes()
{
    return attributes_;
}

} // namespace xmlparser
} // namespace fastrtps
} // namespace eprosima
#endif // !_XML_TREE_