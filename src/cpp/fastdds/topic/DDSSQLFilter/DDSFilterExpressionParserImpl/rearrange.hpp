// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file rearrange.hpp
 *
 * Note: this is an implementation file, designed to be included inside the
 * DDSFilterExpressionParser.hpp file of the parent folder.
 */

// after a node is stored successfully, you can add an optional transformer like this:
struct rearrange
    : parse_tree::apply< rearrange >  // allows bulk selection, see selector<...>
{
    // recursively rearrange nodes. the basic principle is:
    //
    // from:        SEQ_BASED_RULE
    //                /   |   \          (LHS... may be one or more children, followed by OP,)
    //             LHS... OP   RHS       (which is one operator, and RHS, which is a single child)
    //
    // to:               OP
    //                  /  \             (OP now has two children, the original PROD/EXPR and RHS)
    //    SEQ_BASED_RULE    RHS          (Note that PROD/EXPR has two fewer children now)
    //          |
    //         LHS...
    //
    // if only one child is left for LHS..., replace the SEQ_BASED_RULE with the child directly.
    // otherwise, perform the above transformation, then apply it recursively until LHS...
    // becomes a single child, which then replaces the parent node and the recursion ends.
    template< typename ... States >
    static void transform(
            std::unique_ptr< ParseNode >& n,
            States&&... st)
    {
        if (n->children.size() == 1)
        {
            n = std::move(n->children.back());
        }
        else
        {
            n->remove_content();
            auto& c = n->children;
            auto r = std::move(c.back());
            c.pop_back();
            auto o = std::move(c.back());
            c.pop_back();
            if (c.empty())
            {
                o->children.emplace_back(std::move(r));
                n = std::move(o);
            }
            else
            {
                o->children.emplace_back(std::move(n));
                o->children.emplace_back(std::move(r));
                n = std::move(o);
                transform(n->children.front(), st ...);
            }
        }
    }

};
