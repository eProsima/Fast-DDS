// Fast DDS addition to the vendored boost::interprocess.
//
// Validation of intrusive trees that live inside a shared memory segment which
// may have been left inconsistent by an abruptly terminated process.

#ifndef BOOST_INTERPROCESS_DETAIL_TREE_SANITY_HPP
#define BOOST_INTERPROCESS_DETAIL_TREE_SANITY_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif // ifndef BOOST_CONFIG_HPP
#
#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif // if defined(BOOST_HAS_PRAGMA_ONCE)

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>
#include <boost/move/detail/to_raw_pointer.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace boost {
namespace interprocess {
namespace ipcdetail {

//!Checks that every node of an intrusive binary search tree lies inside
//![low, high), is suitably aligned, and that parent/child links are mutually
//!consistent. No node is dereferenced before its address has been validated, so
//!a damaged tree makes this return false instead of faulting.
//!
//!The traversal visits at most as many nodes as could possibly fit in the
//!range, which bounds it even if the links form a cycle.
template<class Container>
bool tree_links_in_range(
        const Container& tree,
        const void* low,
        const void* high)
{
    typedef typename Container::node_traits node_traits;
    typedef typename node_traits::node node_type;
    typedef typename node_traits::const_node_ptr const_node_ptr;

    const std::uintptr_t lo = reinterpret_cast<std::uintptr_t>(low);
    const std::uintptr_t hi = reinterpret_cast<std::uintptr_t>(high);

    if (hi < lo || (hi - lo) < sizeof(node_type))
    {
        return false;
    }

    const std::size_t max_nodes = static_cast<std::size_t>(hi - lo) / sizeof(node_type);

    auto in_range = [lo, hi](const node_type* node)
            {
                const std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(node);
                return addr >= lo && addr <= hi - sizeof(node_type) &&
                       (addr % alignof(node_type)) == 0;
            };

    const_node_ptr header_ptr = tree.cend().pointed_node();
    const node_type* header = &*header_ptr;
    if (!in_range(header))
    {
        return false;
    }

    const node_type* root = boost::movelib::to_raw_pointer(node_traits::get_parent(header_ptr));
    if (nullptr == root)
    {
        // Empty tree
        return true;
    }

    if (!in_range(root) || boost::movelib::to_raw_pointer(node_traits::get_parent(const_node_ptr(root))) != header)
    {
        return false;
    }

    std::vector<const node_type*> pending;
    pending.push_back(root);

    std::size_t visited = 0;
    while (!pending.empty())
    {
        const node_type* node = pending.back();
        pending.pop_back();

        if (++visited > max_nodes)
        {
            return false;
        }

        const const_node_ptr node_ptr(node);
        const node_type* children[2] = {
            boost::movelib::to_raw_pointer(node_traits::get_left(node_ptr)),
            boost::movelib::to_raw_pointer(node_traits::get_right(node_ptr))
        };

        for (const node_type* child : children)
        {
            if (nullptr == child)
            {
                continue;
            }

            if (!in_range(child) ||
                    boost::movelib::to_raw_pointer(node_traits::get_parent(const_node_ptr(child))) != node)
            {
                return false;
            }

            pending.push_back(child);
        }
    }

    return true;
}

} // namespace ipcdetail
} // namespace interprocess
} // namespace boost

#include <boost/interprocess/detail/config_end.hpp>

#endif // BOOST_INTERPROCESS_DETAIL_TREE_SANITY_HPP
