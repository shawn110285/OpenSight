//-----------------------------------------------------------------------------
// File:    osalbasic.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
//-----------------------------------------------------------------------------

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
//-----------------------------------------------------------------------------


#ifndef __YAML_CFG_H__
#define __YAML_CFG_H__

#include <yaml.h>

typedef struct
{
    yaml_document_t *document;
    yaml_node_t *node;
    yaml_node_pair_t *pair;
    yaml_node_item_t *item;
} yaml_iter_t;

extern void  yaml_iter_init(yaml_iter_t *iter, yaml_document_t *document);
extern int   yaml_iter_next(yaml_iter_t *iter);
extern void  yaml_iter_recurse(yaml_iter_t *parent, yaml_iter_t *iter);
extern int   yaml_iter_type(yaml_iter_t *iter);
extern int   yaml_iter_bool(yaml_iter_t *iter);
const char * yaml_iter_key(yaml_iter_t *iter);
const char * yaml_iter_value(yaml_iter_t *iter);


#endif /* __YAML_CFG_H__ */
