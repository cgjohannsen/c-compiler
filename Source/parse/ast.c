void 
init_astnode(astnode_t *node, asttype_t asttype)
{
    node = (astnode_t *) malloc(sizeof(astnode_t));

    node->left = NULL;
    node->right = NULL;
    node->asttype = asttype;
    node->text = "";
    node->type = "";
    node->is_array = false;
    node->is_const = false;
    node->is_struct = false;
}


void 
init_astnode(astnode_t *node)
{
    node = (astnode_t *) malloc(sizeof(astnode_t));

    node->left = NULL;
    node->right = NULL;
    node->text = "";
    node->type = "";
    node->is_array = false;
    node->is_const = false;
    node->is_struct = false;
}


void 
add_astchild(astnode_t *parent, astnode_t *child)
{
    astnode_t *cur;
    cur = parent->left;

    if(cur == NULL) { // case when parent has no children
        parent->left = child;
    }

    while(cur->right != NULL) {
        cur = cur->right;
    }

    cur->right = child;
}


void 
add_astsibling(astnode_t *node, astnode_t *sibling)
{
    astnode_t *cur;
    cur = node->right;

    if(cur == NULL) { // case where node is only child
        node->left = sibling;
    }

    while(cur->right != NULL) {
        cur = cur->right;
    }

    cur->right = sibling;
}


void 
free_ast(astnode_t *node)
{
    if(node == NULL) {
        return;
    }

    free_ast(node->left);
    free_ast(node->right);
    free(node);
}


int 
is_inttype(asttype_t asttype)
{
    return asttype == _CHAR || asttype == _INT;
}


int is_numerictype(asttype_t asttype)
{   
    return asttype == _CHAR || asttype == _INT || asttype == _FLOAT;
}

