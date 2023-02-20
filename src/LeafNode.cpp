#include "RecordPtr.hpp"
#include "LeafNode.hpp"

LeafNode::LeafNode(const TreePtr &tree_ptr) : TreeNode(LEAF, tree_ptr) {
    this->data_pointers.clear();
    this->next_leaf_ptr = NULL_PTR;
    if(!is_null(tree_ptr))
        this->load();
}

//returns max key within this leaf
Key LeafNode::max() {
    auto it = this->data_pointers.rbegin();
    return it->first;
}

//inserts <key, record_ptr> to leaf. If overflow occurs, leaf is split
//split node is returned
//TODO: LeafNode::insert_key to be implemented
TreePtr LeafNode::insert_key(const Key &key, const RecordPtr &record_ptr) {
    TreePtr new_leaf = NULL_PTR; //if leaf is split, new_leaf = ptr to new split node ptr

    bool inserted = false; 

    if(this->data_pointers.size()==0)
    {
        this->data_pointers.insert(pair<Key,RecordPtr>(key, record_ptr));

        inserted=true;
        this->size = this->data_pointers.size();
        this->dump();
        return new_leaf;
    }

    if(this->size == FANOUT)
    {
        // split
        //create a new leaf node
        auto split_node = new LeafNode(new_leaf);

        // insert the key at the correct positon
        for (auto it = this->data_pointers.begin(); it != this->data_pointers.end(); ++it)
        {
            if (key < it->first)
            {
                this->data_pointers.insert(it, {key, record_ptr});
                inserted=true;
                break;
            }
        }

        if(inserted==false) // the case where key is the max of all elements
        {
            this->data_pointers.insert({key, record_ptr});
            inserted=true;
        }
        // now we split the new data_pointer map
        int max_in_left_node = ceil((float)FANOUT/(float)2);
        int cnt=0;
        for (auto it = this->data_pointers.begin(); it != this->data_pointers.end(); ++it)
        {
            if(cnt >= max_in_left_node)
            {
                split_node->data_pointers.insert({it->first, it->second});
            }
            cnt++;
        }
        cnt=0;
        auto it = this->data_pointers.begin();
        while(cnt<max_in_left_node)
        {
            it++;
            cnt++;
        }
        this->data_pointers.erase(it, this->data_pointers.end());
        split_node->size = split_node->data_pointers.size();
        new_leaf = split_node->tree_ptr;
        TreePtr temp = this->next_leaf_ptr;
        this->next_leaf_ptr = new_leaf;
        split_node->next_leaf_ptr = temp;
        split_node->dump();
    }
    else
    {
        for (auto it = this->data_pointers.begin(); it != this->data_pointers.end(); ++it) 
        {
            if (key < it->first)
            {
                this->data_pointers.insert(it, {key, record_ptr});
                inserted=true;
                break;
                /* code */
            }
        }
        if(inserted==false)
        {
            this->data_pointers.insert({key, record_ptr});
            inserted=true;
        }
    }
    this->size = this->data_pointers.size();
    this->dump();
    return new_leaf;
}

//key is deleted from leaf if exists
//TODO: LeafNode::delete_key to be implemented
void LeafNode::delete_key(const Key &key) {     
    this->data_pointers.erase(key); 
    this->size = this->data_pointers.size();    
    this->dump();
}

void LeafNode::merge_siblings(const TreePtr &tree_ptr)
{
    auto temp_tree_ptr = new LeafNode(tree_ptr);

    for (auto it = temp_tree_ptr->data_pointers.begin(); it != temp_tree_ptr->data_pointers.end(); ++it)
    {
        this->data_pointers.insert({it->first, it->second});
    }
    this->size = this->data_pointers.size();
    this->next_leaf_ptr = temp_tree_ptr->next_leaf_ptr;
    this->dump();
}

void LeafNode::redistribute_siblings(const TreePtr &tree_ptr, int check)
{
    auto temp_tree_ptr = new LeafNode(tree_ptr);

    if(check==0) // left sibling redistribute for this : tree ptr is left sibling to this
    {
        // writing elements from this to sibling
        for (auto it = this->data_pointers.begin(); it != this->data_pointers.end(); ++it)
        {
            temp_tree_ptr->data_pointers.insert({it->first, it->second});
        } 

        // writting F/2 elements back to this : underflowed
        temp_tree_ptr->size = temp_tree_ptr->data_pointers.size();
        
        this->data_pointers.clear();
        int ele_required_in_temp_tree = temp_tree_ptr->size - MIN_OCCUPANCY;
        int cnt=1;
        
        for (auto it = temp_tree_ptr->data_pointers.begin(); it != temp_tree_ptr->data_pointers.end(); ++it)
        {
            if(cnt > ele_required_in_temp_tree)
                this->data_pointers.insert({it->first, it->second});
            cnt++;
        }

        cnt=1;
        for (auto it = temp_tree_ptr->data_pointers.begin(); it != temp_tree_ptr->data_pointers.end(); ++it)
        {
            if(cnt > ele_required_in_temp_tree)
                {
                    temp_tree_ptr->data_pointers.erase(it, temp_tree_ptr->data_pointers.end());
                    break;
                }
            cnt++;
        }
    }

    else // right sibling redistribute for this : tree ptr is right sibling to this
    {
        // writing elements from sibling to this
        for (auto it = temp_tree_ptr->data_pointers.begin(); it != temp_tree_ptr->data_pointers.end(); ++it)
        {
            this->data_pointers.insert({it->first, it->second});
        } 

        // writting F/2 elements back to this : underflowed
        this->size = this->data_pointers.size();
        temp_tree_ptr->data_pointers.clear();
        int ele_required_in_temp_tree = this->size - MIN_OCCUPANCY;
        int cnt=1;
        
        for (auto it = this->data_pointers.begin(); it != this->data_pointers.end(); ++it)
        {
            if(cnt > MIN_OCCUPANCY)
                temp_tree_ptr->data_pointers.insert({it->first, it->second});
            cnt++;
        }

        cnt=1;
        for (auto it = this->data_pointers.begin(); it != this->data_pointers.end(); ++it)
        {
            if(cnt > MIN_OCCUPANCY)
                {
                    this->data_pointers.erase(it, this->data_pointers.end());
                    break;
                }
            cnt++;
        }
    }
    
    this->size = this->data_pointers.size();
    temp_tree_ptr->size = temp_tree_ptr->data_pointers.size();
    
    this->dump();
    temp_tree_ptr->dump();
}

//runs range query on leaf
void LeafNode::range(ostream &os, const Key &min_key, const Key &max_key) const {
    BLOCK_ACCESSES++;
    for(const auto& data_pointer : this->data_pointers){
        if(data_pointer.first >= min_key && data_pointer.first <= max_key)
            data_pointer.second.write_data(os);
        if(data_pointer.first > max_key)
            return;
    }
    if(!is_null(this->next_leaf_ptr)){
        auto next_leaf_node = new LeafNode(this->next_leaf_ptr);
        next_leaf_node->range(os, min_key, max_key);
        delete next_leaf_node;
    }
}

//exports node - used for grading
void LeafNode::export_node(ostream &os) {
    TreeNode::export_node(os);
    for(const auto& data_pointer : this->data_pointers){
        os << data_pointer.first << " ";
    }
    os << endl;
}

//writes leaf as a mermaid chart
void LeafNode::chart(ostream &os) {
    string chart_node = this->tree_ptr + "[" + this->tree_ptr + BREAK;
    chart_node += "size: " + to_string(this->size) + BREAK;
    for(const auto& data_pointer: this->data_pointers) {
        chart_node += to_string(data_pointer.first) + " ";
    }
    chart_node += "]";
    os << chart_node << endl;
}

ostream& LeafNode::write(ostream &os) const {
    TreeNode::write(os);
    for(const auto & data_pointer : this->data_pointers){
        if(&os == &cout)
            os << "\n" << data_pointer.first << ": ";
        else
            os << "\n" << data_pointer.first << " ";
        os << data_pointer.second;
    }
    os << endl;
    os << this->next_leaf_ptr << endl;
    return os;
}

istream& LeafNode::read(istream& is){
    TreeNode::read(is);
    this->data_pointers.clear();
    for(int i = 0; i < this->size; i++){
        Key key = DELETE_MARKER;
        RecordPtr record_ptr;
        if(&is == &cin)
            cout << "K: ";
        is >> key;
        if(&is == &cin)
            cout << "P: ";
        is >> record_ptr;
        this->data_pointers.insert(pair<Key,RecordPtr>(key, record_ptr));
    }
    is >> this->next_leaf_ptr;
    return is;
}