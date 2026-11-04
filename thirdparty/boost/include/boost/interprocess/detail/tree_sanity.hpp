//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2006-2012. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_DETAIL_TREE_SANITY_HPP
#define BOOST_INTERPROCESS_DETAIL_TREE_SANITY_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif
#
#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>
#include <boost/move/detail/to_raw_pointer.hpp>
#include <boost/move/detail/type_traits.hpp> //alignment_of

#include <cstddef>
#include <vector>

//!\file
//!Validation of intrusive trees that live inside a shared memory segment which
//!may have been left inconsistent by an abruptly terminated process.

namespace boost {
namespace interprocess {
namespace ipcdetail {

template<class NodeType>
inline bool tree_node_in_range(const NodeType *node, std::size_t lo, std::size_t hi)
{
   const std::size_t addr = reinterpret_cast<std::size_t>(node);
   return addr >= lo && addr <= hi - sizeof(NodeType) &&
          (addr % ::boost::container::dtl::alignment_of<NodeType>::value) == 0;
}

//!Checks that every node of an intrusive binary search tree lies inside
//![low, high), is suitably aligned, and that parent/child links are mutually
//!consistent. No node is dereferenced before its address has been validated, so
//!a damaged tree makes this return false instead of faulting.
//!
//!The traversal visits at most as many nodes as could possibly fit in the
//!range, which bounds it even if the links form a cycle.
template<class Container>
bool tree_links_in_range(const Container &tree, const void *low, const void *high)
{
   typedef typename Container::node_traits      node_traits;
   typedef typename node_traits::node           node_type;
   typedef typename node_traits::const_node_ptr const_node_ptr;

   const std::size_t lo = reinterpret_cast<std::size_t>(low);
   const std::size_t hi = reinterpret_cast<std::size_t>(high);

   if(hi < lo || (hi - lo) < sizeof(node_type)){
      return false;
   }

   const std::size_t max_nodes = (hi - lo) / sizeof(node_type);

   const_node_ptr header_ptr = tree.cend().pointed_node();
   const node_type *header = &*header_ptr;
   if(!tree_node_in_range(header, lo, hi)){
      return false;
   }

   const node_type *root = boost::movelib::to_raw_pointer(node_traits::get_parent(header_ptr));
   if(!root){
      //Empty tree
      return true;
   }

   if(!tree_node_in_range(root, lo, hi) ||
      boost::movelib::to_raw_pointer(node_traits::get_parent(const_node_ptr(root))) != header){
      return false;
   }

   std::vector<const node_type*> pending;
   pending.push_back(root);

   std::size_t visited = 0;
   while(!pending.empty()){
      const node_type *node = pending.back();
      pending.pop_back();

      if(++visited > max_nodes){
         return false;
      }

      const const_node_ptr node_ptr(node);
      const node_type *children[2] = {
         boost::movelib::to_raw_pointer(node_traits::get_left(node_ptr)),
         boost::movelib::to_raw_pointer(node_traits::get_right(node_ptr))
      };

      for(std::size_t i = 0; i != 2; ++i){
         const node_type *child = children[i];
         if(!child){
            continue;
         }

         if(!tree_node_in_range(child, lo, hi) ||
            boost::movelib::to_raw_pointer(node_traits::get_parent(const_node_ptr(child))) != node){
            return false;
         }

         pending.push_back(child);
      }
   }

   return true;
}

}  //namespace ipcdetail {
}  //namespace interprocess {
}  //namespace boost {

#include <boost/interprocess/detail/config_end.hpp>

#endif //#ifndef BOOST_INTERPROCESS_DETAIL_TREE_SANITY_HPP
