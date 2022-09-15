

#include "../include/osalyaml.h"
#include "../../osal/include/osalbasic.h"

void yaml_iter_init(yaml_iter_t *iter, yaml_document_t *document)
{
    if(!iter)
    {
        printf("iter is invalid, File:%s, line:%d \r\n", __FILE__, __LINE__);
        return;
    }

    if(!document)
    {
        printf("document is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return;
    }

    memset(iter, 0, sizeof(yaml_iter_t));

    iter->document = document;
    iter->node = yaml_document_get_root_node(document);

    if(!iter->node)
    {
        printf("iter->node is invalid, File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return;
    }

    if (iter->node->type == YAML_MAPPING_NODE)
        iter->pair = iter->node->data.mapping.pairs.start - 1;
    else if (iter->node->type == YAML_SEQUENCE_NODE)
        iter->item = iter->node->data.sequence.items.start - 1;
}

int yaml_iter_next(yaml_iter_t *iter)
{
    if(!iter)
    {
        printf("iter is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return 0;
    }

    if(!(iter->document))
    {
        printf("document is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return 0;
    }

    if(!(iter->node))
    {
        printf("iter->node is invalid,  File:%s, line:%d \r\n", __FILE__, __LINE__);
        return 0;
    }

    if (iter->node->type == YAML_MAPPING_NODE)
    {
        if (iter->pair)
        {
            iter->pair++;
            if (iter->pair < iter->node->data.mapping.pairs.top)
                return 1;
        }

    }
    else if (iter->node->type == YAML_SEQUENCE_NODE)
    {
        if (iter->item)
        {
            iter->item++;
            if (iter->item < iter->node->data.sequence.items.top)
                return 1;
        }
    }

    return 0;
}

void yaml_iter_recurse(yaml_iter_t *parent, yaml_iter_t *iter)
{
    if(!parent)
    {
        printf("parent is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return;
    }

    if(! (parent->document))
    {
        printf("parent->document is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return;
    }

    if(!(parent->node))
    {
        printf("parent->node is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return;
    }

    if(!iter)
    {
        printf("iter is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return;
    }

    memset(iter, 0, sizeof(yaml_iter_t));

    iter->document = parent->document;

    if (parent->node->type == YAML_MAPPING_NODE)
    {
        if(!parent->pair)
        {
            printf("parent->pair is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return;
        }
        iter->node = yaml_document_get_node(parent->document, parent->pair->value);

        if(!(iter->node))
        {
            printf("iter->node is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return;
        }

        if (iter->node->type == YAML_MAPPING_NODE)
            iter->pair = iter->node->data.mapping.pairs.start - 1;
        else if (iter->node->type == YAML_SEQUENCE_NODE)
            iter->item = iter->node->data.sequence.items.start - 1;
    }
    else if (parent->node->type == YAML_SEQUENCE_NODE)
    {
        if(!(parent->item))
        {
            printf("parent->item is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return;
        }

        iter->node = yaml_document_get_node(parent->document, *parent->item);

        if(!(iter->node))
        {
            printf("iter->node is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return;
        }
        if (iter->node->type == YAML_MAPPING_NODE)
            iter->pair = iter->node->data.mapping.pairs.start - 1;
        else if (iter->node->type == YAML_SEQUENCE_NODE)
            iter->item = iter->node->data.sequence.items.start - 1;
    }
    else
    {
        printf("unknow node type,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return;
    }
}

int yaml_iter_type(yaml_iter_t *iter)
{
    if(!iter)
    {
        printf("iter is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return 0;
    }

    if(!iter->node)
    {
        printf("iter->node is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return 0;
    }

    return iter->node->type;
}


const char *yaml_iter_key(yaml_iter_t *iter)
{
    yaml_node_t *node = NULL;

    if(!iter)
    {
        printf("iter is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return NULL;
    }

    if(!iter->document)
    {
        printf("iter->document is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return NULL;
    }

    if(!iter->node)
    {
        printf("iter->node is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return NULL;
    }

    if (iter->node->type == YAML_MAPPING_NODE)
    {
        if(!iter->pair)
        {
            printf("iter->pair is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return NULL;
        }
        node = yaml_document_get_node(iter->document, iter->pair->key);
        if(!node)
        {
            printf("node is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return NULL;
        }
        if(node->type != YAML_SCALAR_NODE)
        {
            printf("node->type != YAML_SCALAR_NODE,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return NULL;
        }
        return (const char *)node->data.scalar.value;
    }
    else if (iter->node->type == YAML_SEQUENCE_NODE)
    {
        if(!iter->item)
        {
            printf("iter is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return NULL;
        }

        node = yaml_document_get_node(iter->document, *iter->item);
        if(!node)
        {
            printf("node is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return NULL;
        }

        if(node->type != YAML_SCALAR_NODE)
        {
            printf("node->type != YAML_SCALAR_NODE,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return NULL;
        }

        return (const char *)node->data.scalar.value;
    }
    else
    {
        printf("unknow node type,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return NULL;
    }
    return NULL;
}

const char *yaml_iter_value(yaml_iter_t *iter)
{
    if(!iter)
    {
        printf("iter is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return NULL;
    }

    if(!iter->document)
    {
        printf("iter->document is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return NULL;
    }

    if(!iter->node)
    {
        printf("iter->node is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return NULL;
    }

    if (iter->node->type == YAML_SCALAR_NODE)
    {
        return (const char *)iter->node->data.scalar.value;
    }
    else if (iter->node->type == YAML_MAPPING_NODE)
    {
        yaml_node_t *node = NULL;

        if(!iter->pair)
        {
            printf("iter->pair is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return NULL;
        }

        node = yaml_document_get_node(iter->document, iter->pair->value);
        if(!node)
        {
            printf("node is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return NULL;
        }

        if(node->type != YAML_SCALAR_NODE)
        {
            printf("node->type != YAML_SCALAR_NODE,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return NULL;
        }
        return (const char *)node->data.scalar.value;
    }
    else if (iter->node->type == YAML_SEQUENCE_NODE)
    {
        yaml_node_t *node = NULL;

        if(!iter->item)
        {
            printf("iter->pair is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return NULL;
        }

        node = yaml_document_get_node(iter->document, *iter->item);
        if(!node)
        {
            printf("node is invalid,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return NULL;
        }

        if(node->type != YAML_SCALAR_NODE)
        {
            printf("node->type != YAML_SCALAR_NODE,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return NULL;
        }

        return (const char *)node->data.scalar.value;
    }
    else
    {
        printf("unknow node type,  File:%s, line:%d  \r\n", __FILE__, __LINE__);
        return NULL;
    }
    return NULL;
}


int yaml_iter_bool(yaml_iter_t *iter)
{
    const char *v = yaml_iter_value(iter);
    if (v)
    {
        if (!strcasecmp(v, "true") || !strcasecmp(v, "yes")) return 1;
        if (atoi(v)) return 1;
    }

    return 0;
}
