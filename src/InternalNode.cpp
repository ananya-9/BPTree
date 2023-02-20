#include "InternalNode.hpp"

//creates internal node pointed to by tree_ptr or creates a new one
InternalNode::InternalNode(const TreePtr &tree_ptr) : TreeNode(INTERNAL, tree_ptr) {
    this->keys.clear();
    this->tree_pointers.clear();
    if(!is_null(tree_ptr))
        this->load();
}

//max element from tree rooted at this node
Key InternalNode::max() {
    Key max_key = DELETE_MARKER;
    TreeNode* last_tree_node = TreeNode::tree_node_factory(this->tree_pointers[this->size - 1]);
    max_key = last_tree_node->max();
    delete last_tree_node;
    return max_key;
}

//if internal node contains a single child, it is returned
TreePtr InternalNode::single_child_ptr() {
    if(this->size == 1)
        return this->tree_pointers[0];
    return NULL_PTR;
}

Key InternalNode::get_key() {    
    return this->keys[0];
}

void InternalNode::push_key(Key key) {
    this->keys.push_back(key);
}

void InternalNode::del_key() {
    this->keys.pop_back();
}
//inserts <key, record_ptr> into subtree rooted at this node.
//returns pointer to split node if exists
//TODO: InternalNode::insert_key to be implemented
TreePtr InternalNode::insert_key(const Key &key, const RecordPtr &record_ptr) {
    TreePtr new_tree_ptr = NULL_PTR;
    TreePtr potential_split_child_ptr = NULL_PTR;
    TreePtr child_ptr = NULL_PTR;
    int ind_of_chil_ptr=0;
    int no_of_keys = this->keys.size();
    if(key <= this->keys[0])
    {    
        child_ptr = this->tree_pointers[0];
        ind_of_chil_ptr=0;
    }

    // checking for last tree pointer : most right
    else if(key > this->keys[no_of_keys-1])
    {
        child_ptr = this->tree_pointers[no_of_keys];
        ind_of_chil_ptr=no_of_keys;
    }

    // checing for all the remaining tree pointers in between
    for(auto i=0; i<no_of_keys-1;i++)
    {
        if(key>this->keys[i] and key <= this->keys[i+1])
        {
            child_ptr = this->tree_pointers[i+1];
            ind_of_chil_ptr=i+1;
            break;
        }
    }

    TreeNode* temp_tree_ptr = TreeNode::tree_node_factory(child_ptr);
    potential_split_child_ptr = temp_tree_ptr->insert_key(key, record_ptr);
    // checking if the child returns a non-null ptr, 
    // i.e. and element needs to be added to the current internal node
    if(!is_null(potential_split_child_ptr))
    {
        // INSERT THE SPLIT POINTER NEXT TO CHILD POINTER
        // the value pushed up, is inserted at child_ptr_index, 
        // and the new potential_split_ptr recieved, is inserted at child_ptr_indx+1
        this->keys.insert(this->keys.begin() + ind_of_chil_ptr, temp_tree_ptr->max());
        this->tree_pointers.insert(this->tree_pointers.begin() + ind_of_chil_ptr + 1, potential_split_child_ptr);
        this->size = this->tree_pointers.size();
    }

    // checking if this insertion will make current node
    // split
    if(this->size > FANOUT)
    {
        InternalNode* split_node = new InternalNode(new_tree_ptr);

        int max_in_left_node = ceil((float)FANOUT/(float)2);
        int cnt=0;


        //Inserting fata in the new split_ptr 
        for (auto i = 0; i != this->keys.size(); i++)
        {
            if(cnt >= max_in_left_node)
            {
                split_node->keys.push_back(this->keys[i]);
                split_node->tree_pointers.push_back(this->tree_pointers[i]);
            }
            cnt++;
        }
        split_node->tree_pointers.push_back(this->tree_pointers[this->keys.size()]);


        // Delete the stuff from current tree_ptr
        this->keys.erase(this->keys.begin()+max_in_left_node, this->keys.end());
        this->tree_pointers.erase(this->tree_pointers.begin()+max_in_left_node, this->tree_pointers.end());

        split_node->size = split_node->tree_pointers.size();
        new_tree_ptr = split_node->tree_ptr;
        split_node->dump();
    }

    this->size = this->tree_pointers.size();
    this->dump();
    return new_tree_ptr;
}

//deletes key from subtree rooted at this if exists
//TODO: InternalNode::delete_key to be implemented
void InternalNode::delete_key(const Key &key) {
    TreePtr new_tree_ptr = NULL_PTR;
    TreePtr child_ptr = NULL_PTR;
    int ind_of_chil_ptr=0;
    int no_of_keys = this->keys.size();
    if(key <= this->keys[0])
    {    
        child_ptr = this->tree_pointers[0];
        ind_of_chil_ptr=0;
    }

    // checking for last tree pointer : most right
    else if(key > this->keys[no_of_keys-1])
    {
        child_ptr = this->tree_pointers[no_of_keys];
        ind_of_chil_ptr=no_of_keys;
    }

    // checing for all the remaining tree pointers in between
    for(auto i=0; i<no_of_keys-1;i++)
    {
        if(key>this->keys[i] and key <= this->keys[i+1])
        {
            child_ptr = this->tree_pointers[i+1];
            ind_of_chil_ptr=i+1;
            break;
        }
    }

    TreeNode* temp_tree_ptr = TreeNode::tree_node_factory(child_ptr);
    temp_tree_ptr->delete_key(key);
    if(key == this->keys[ind_of_chil_ptr])
    {
        this->keys.erase(this->keys.begin() + ind_of_chil_ptr);
        this->keys.insert(this->keys.begin() + ind_of_chil_ptr, temp_tree_ptr->max());
    }
    
    /*
    PREFERENCE ORDER FOR DELETE_KEY
    If left sibling exists, and redistribution can occur - perform redistribution with left sibling else
    If left sibling exists, and merging can occur - perform merge with left sibling else
    If right sibling exists, and redistribution can occur - perform redistribution with right sibling else
    If right sibling exists, and merging can occur - perform merge with right sibling
    */

    // Any kind of rearrangement occurs only when there's an underflow in the child_ptr
    // condition for underflow : child_ptr->
    int underflow_check = ceil((float)FANOUT/(float)2);
    bool rearrangement = false;

    bool left_sibling_exist = false;
    bool right_sibling_exist = false;

    if(ind_of_chil_ptr>0)
        left_sibling_exist=true;
    if(ind_of_chil_ptr < this->tree_pointers.size()-1)
        right_sibling_exist=true;


    if(temp_tree_ptr->size < underflow_check)
    {
        if(left_sibling_exist)
        {
            // creating the left sibling
            TreeNode* left_sibling = TreeNode::tree_node_factory(this->tree_pointers[ind_of_chil_ptr-1]);
            
            // Redistribution with left : left_siblilng_ptr->size + temp_tree_ptr->size > 2 * ceil((float)FANOUT/(float(2)));
            if(left_sibling->size + temp_tree_ptr->size >= 2 * underflow_check)
            {   
                temp_tree_ptr->redistribute_siblings(left_sibling->tree_ptr, 0);
                left_sibling->load();
                this->keys[ind_of_chil_ptr-1] = left_sibling->max();
                rearrangement = true;
            }

            // Merge with left : left_sibling->size + temp_tree_ptr->size < Max_element_in_a_node
            else if(left_sibling->size + temp_tree_ptr->size <= FANOUT and rearrangement == false)
            {
                //Key left_sibling_max = left_sibling->max(); //storing the current max for the left sibling in tem[p variable. can be used in edge case

                left_sibling->merge_siblings(temp_tree_ptr->tree_ptr); // temp_tree_ptr object was created using child_ptr only i.e. the current sibling
                this->keys[ind_of_chil_ptr-1] = left_sibling->max();

                if(this->size>2)
                    this->keys.erase(this->keys.begin() + ind_of_chil_ptr-1);

                this->tree_pointers.erase(this->tree_pointers.begin() + ind_of_chil_ptr);
                this->size = this->tree_pointers.size();

                temp_tree_ptr->delete_node();
                rearrangement = true;
                
                
            }
        }
        if(right_sibling_exist and rearrangement == false)
        {
            // creating the right sibling
            TreeNode* right_sibling = TreeNode::tree_node_factory(this->tree_pointers[ind_of_chil_ptr+1]);
            
            // Redistribution with right
            if(right_sibling->size + temp_tree_ptr->size >= 2 * underflow_check)
            {
                temp_tree_ptr->redistribute_siblings(right_sibling->tree_ptr, 1);
                this->keys[ind_of_chil_ptr] = temp_tree_ptr->max();
                rearrangement = true;
            }

            // Merge with right
            else if(right_sibling->size + temp_tree_ptr->size <= FANOUT and rearrangement == false)
            {
                temp_tree_ptr->merge_siblings(right_sibling->tree_ptr);

                // After merging siblings, update current internal nodes keys and tree pointers
                this->keys[ind_of_chil_ptr] = temp_tree_ptr->max();
                if(this->size>2)
                    this->keys.erase(this->keys.begin() + ind_of_chil_ptr+1);

                this->tree_pointers.erase(this->tree_pointers.begin() + ind_of_chil_ptr + 1);
                this->size = this->tree_pointers.size();
                right_sibling->delete_node();
                rearrangement = true;
            }
        }   
    }

    this->dump();
}

void InternalNode::merge_siblings(const TreePtr &tree_ptr)
{

    this->keys.push_back(this->max());
    if(this->size==1)
        this->keys.pop_back();
    auto temp_tree_ptr = new InternalNode(tree_ptr);

    for (auto i = 0; i != temp_tree_ptr->keys.size(); i++)
        this->keys.push_back(temp_tree_ptr->keys[i]);
    
    for (auto i = 0; i != temp_tree_ptr->tree_pointers.size(); i++)
        this->tree_pointers.push_back(temp_tree_ptr->tree_pointers[i]);
    
    this->size = this->tree_pointers.size();
    this->dump();
}
void InternalNode::redistribute_siblings(const TreePtr &tree_ptr, int check)
{
    auto temp_tree_ptr = new InternalNode(tree_ptr);

    if(check==0) // left sibling merge for this : tree ptr is left sibling to this
    {
        temp_tree_ptr->keys.push_back(temp_tree_ptr->max());

        // writing elements from this to sibling
        for (auto it = this->tree_pointers.begin(); it != this->tree_pointers.end(); ++it)
            temp_tree_ptr->tree_pointers.push_back(*it);

        for (auto it = this->keys.begin(); it != this->keys.end(); ++it)
            temp_tree_ptr->keys.push_back(*it);

        temp_tree_ptr->size = temp_tree_ptr->tree_pointers.size();
        this->tree_pointers.clear();        
        this->keys.clear();

        if(FANOUT==3)
            temp_tree_ptr->keys.pop_back();

        // writting F/2 elements back to this : underflowed
        int ele_required_in_temp_tree = temp_tree_ptr->size - MIN_OCCUPANCY;
        int cnt=1;
        for (auto it = temp_tree_ptr->tree_pointers.begin(); it != temp_tree_ptr->tree_pointers.end(); ++it)
        {
            if(cnt > ele_required_in_temp_tree)
                this->tree_pointers.push_back(*it);
            cnt++;
        }
        cnt=1;
        for (auto it = temp_tree_ptr->keys.begin(); it != temp_tree_ptr->keys.end(); ++it)
        {
            if(cnt > ele_required_in_temp_tree)
                this->keys.push_back(*it);
            cnt++;
        }

        // DELETING EXTRA STUFF FROM TEMPTREEPTR
        for(int i=0;i<MIN_OCCUPANCY;i++)
        {
            temp_tree_ptr->tree_pointers.pop_back();
            temp_tree_ptr->keys.pop_back();
        }

    }

    else // right sibling merge for this : tree ptr is right sibling to this
    {
        if(FANOUT>3)
            this->keys.push_back(this->max());

        // writing elements from sibling to this

        for (auto it = temp_tree_ptr->tree_pointers.begin(); it != temp_tree_ptr->tree_pointers.end(); ++it)
            this->tree_pointers.push_back(*it);

        for (auto it = temp_tree_ptr->keys.begin(); it != temp_tree_ptr->keys.end(); ++it)
            this->keys.push_back(*it);

        this->size = this->tree_pointers.size();

        // writting F/2 elements back to this : underflowed
        temp_tree_ptr->tree_pointers.clear();
        temp_tree_ptr->keys.clear();
        int ele_required_in_temp_tree = this->size - MIN_OCCUPANCY;
        
        int cnt=1;
        for (auto it = this->tree_pointers.begin(); it != this->tree_pointers.end(); ++it)
        {
            if(cnt > MIN_OCCUPANCY)
                temp_tree_ptr->tree_pointers.push_back(*it);
            cnt++;
        }

        cnt=1;
        for (auto it = this->keys.begin(); it != this->keys.end(); ++it)
        {
            if(cnt > MIN_OCCUPANCY)
                temp_tree_ptr->keys.push_back(*it);
            cnt++;
        }

        // deleting the values from this->tree_ptrs and this->keys
        for(int i=0; i<ele_required_in_temp_tree;i++)
        {
            this->tree_pointers.pop_back();
            this->keys.pop_back();
        }
        
    }


    this->size = this->tree_pointers.size();
    temp_tree_ptr->size = temp_tree_ptr->tree_pointers.size();
    this->dump();
    temp_tree_ptr->dump();
}

//runs range query on subtree rooted at this node
void InternalNode::range(ostream &os, const Key &min_key, const Key &max_key) const {
    BLOCK_ACCESSES++;
    for(int i = 0; i < this->size - 1; i++){
        if(min_key <= this->keys[i]){
            auto* child_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
            child_node->range(os, min_key, max_key);
            delete child_node;
            return;
        }
    }
    auto* child_node = TreeNode::tree_node_factory(this->tree_pointers[this->size - 1]);
    child_node->range(os, min_key, max_key);
    delete child_node;
}

//exports node - used for grading
void InternalNode::export_node(ostream &os) {
    TreeNode::export_node(os);
    for(int i = 0; i < this->size - 1; i++)
        os << this->keys[i] << " ";
    os << endl;
    for(int i = 0; i < this->size; i++){
        auto child_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
        child_node->export_node(os);
        delete child_node;
    }
}

//writes subtree rooted at this node as a mermaid chart
void InternalNode::chart(ostream &os) {
    string chart_node = this->tree_ptr + "[" + this->tree_ptr + BREAK;
    chart_node += "size: " + to_string(this->size) + BREAK;
    chart_node += "]";
    os << chart_node << endl;

    for(int i = 0; i < this->size; i++) {
        auto tree_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
        tree_node->chart(os);
        delete tree_node;
        string link = this->tree_ptr + "-->|";

        if(i == 0)
            link += "x <= " + to_string(this->keys[i]);
        else if (i == this->size - 1) {
            link += to_string(this->keys[i-1]) + " < x";
        } else {
            link += to_string(this->keys[i-1]) + " < x <= " + to_string(this->keys[i]);
        }
        link += "|" + this->tree_pointers[i];
        os << link << endl;
    }
}

ostream& InternalNode::write(ostream &os) const {
    TreeNode::write(os);
    for(int i = 0; i < this->size - 1; i++){
        if(&os == &cout)
            os << "\nP" << i+1 << ": ";
        os << this->tree_pointers[i] << " ";
        if(&os == &cout)
            os << "\nK" << i+1 << ": ";
        os << this->keys[i] << " ";
    }
    if(&os == &cout)
        os << "\nP" << this->size << ": ";
    os << this->tree_pointers[this->size - 1];
    return os;
}

istream& InternalNode::read(istream& is){
    TreeNode::read(is);
    this->keys.assign(this->size - 1, DELETE_MARKER);
    this->tree_pointers.assign(this->size, NULL_PTR);
    for(int i = 0; i < this->size - 1; i++){
        if(&is == &cin)
            cout << "P" << i+1 << ": ";
        is >> this->tree_pointers[i];
        if(&is == &cin)
            cout << "K" << i+1 << ": ";
        is >> this->keys[i];
    }
    if(&is == &cin)
        cout << "P" << this->size;
    is >> this->tree_pointers[this->size - 1];
    return is;
}
