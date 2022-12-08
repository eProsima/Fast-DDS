#include <fastrtps/xmlparser/XMLTree.h>

#include <vector>
#include <string>

#include <gtest/gtest.h>

using namespace eprosima::fastrtps::xmlparser;

using namespace ::testing;


class XMLTreeTests : public ::testing::Test
{
public:

    XMLTreeTests()
    {
    }

    ~XMLTreeTests()
    {
    }

};

TEST_F(XMLTreeTests, OnlyRoot)
{
    BaseNode test_base{NodeType::ROOT};
    ASSERT_EQ(NodeType::ROOT, test_base.getType());
    ASSERT_EQ(false, test_base.removeChild(0));
    ASSERT_EQ(nullptr, test_base.getChild(0));
    ASSERT_EQ(nullptr, test_base.getParent());
    ASSERT_EQ(0u, test_base.getNumChildren());
}

TEST_F(XMLTreeTests, RootChildren)
{
    BaseNode test_base{NodeType::ROOT};

    ASSERT_EQ(false, test_base.removeChild(0));
    ASSERT_EQ(nullptr, test_base.getChild(0));
    ASSERT_EQ(0u, test_base.getNumChildren());

    BaseNode* child = new BaseNode{NodeType::APPLICATION};
    test_base.addChild(std::unique_ptr<BaseNode>(child));

    ASSERT_EQ(1u, test_base.getNumChildren());
    ASSERT_EQ(child, test_base.getChild(0));
    ASSERT_EQ(&test_base, test_base.getChild(0)->getParent());
    ASSERT_EQ(NodeType::APPLICATION, test_base.getChild(0)->getType());
    ASSERT_EQ(true, test_base.removeChild(0));
    ASSERT_EQ(0u, test_base.getNumChildren());
    ASSERT_EQ(false, test_base.removeChild(0));
}

TEST_F(XMLTreeTests, RootMultipleChildren)
{
    const unsigned int num_children = 10;
    BaseNode test_base{NodeType::ROOT};

    ASSERT_EQ(false, test_base.removeChild(0));
    ASSERT_EQ(nullptr, test_base.getChild(0));
    ASSERT_EQ(0u, test_base.getNumChildren());

    std::vector<BaseNode*> children_backup;
    for (unsigned int i = 0; i < num_children; ++i)
    {
        children_backup.push_back(new BaseNode{NodeType::APPLICATION});
        test_base.addChild(std::unique_ptr<BaseNode>(children_backup[i]));
    }

    for (unsigned int i = 0; i < num_children; ++i)
    {
        ASSERT_EQ(children_backup[i], test_base.getChild(i));
        ASSERT_EQ(&test_base, test_base.getChild(i)->getParent());
        ASSERT_EQ(NodeType::APPLICATION, test_base.getChild(i)->getType());
    }

    for (unsigned int i = 0; i < num_children; ++i)
    {
        ASSERT_EQ(num_children - i, test_base.getNumChildren());
        ASSERT_EQ(true, test_base.removeChild(0));
        ASSERT_EQ(num_children - i - 1, test_base.getNumChildren());
        ASSERT_EQ(false, test_base.removeChild(num_children - i));
    }
}

TEST_F(XMLTreeTests, DataNode)
{
    const std::string attribute_name0{"Attribute0"};
    const std::string attribute_name1{"Attribute1"};
    const std::string attribute_value0{"TESTATTRIBUTE"};
    const std::string attribute_value1{"TESTATTRIBUTE1"};

    DataNode<std::string> data_node{NodeType::PUBLISHER};
    std::string* data = new std::string("TESTDATA");

    data_node.setData(std::unique_ptr<std::string>(data));
    data_node.addAttribute(attribute_name0, attribute_value0);
    data_node.addAttribute(attribute_name1, attribute_value1);

    ASSERT_EQ(NodeType::PUBLISHER, data_node.getType());
    ASSERT_EQ(data, data_node.getData().get());
    ASSERT_STREQ(attribute_value0.data(), data_node.getAttributes().at(attribute_name0).data());
    ASSERT_STREQ(attribute_value1.data(), data_node.getAttributes().at(attribute_name1).data());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
