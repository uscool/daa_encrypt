#ifndef HUFFMAN_HPP
#define HUFFMAN_HPP

#include<queue>
#include<unordered_map>
#include<vector>
#include<string>
#include"avl_tree.hpp"

struct HuffmanNode{
    std::string word;
    int frequency;
    HuffmanNode*left;
    HuffmanNode*right;

    HuffmanNode(const std::string&w,int freq):
        word(w),frequency(freq),left(nullptr),right(nullptr){}
};

struct CompareNodes{
    bool operator()(HuffmanNode*a,HuffmanNode*b){
        return a->frequency>b->frequency;
    }
};

class HuffmanCoding{
private:
    HuffmanNode*root;
    std::unordered_map<std::string,std::string>huffmanCodes;

    void generateCodes(HuffmanNode*node,const std::string&code){
        if(node==nullptr)return;

        if(!node->word.empty()){
            huffmanCodes[node->word]=code;
        }

        generateCodes(node->left,code+"0");
        generateCodes(node->right,code+"1");
    }

    void cleanup(HuffmanNode*node){
        if(node!=nullptr){
            cleanup(node->left);
            cleanup(node->right);
            delete node;
        }
    }

    void collectNodes(AVLNode*avlNode,std::vector<HuffmanNode*>&nodes){
        if(avlNode==nullptr)return;

        collectNodes(avlNode->left,nodes);
        nodes.push_back(new HuffmanNode(avlNode->word,avlNode->count));
        collectNodes(avlNode->right,nodes);
    }

public:
    HuffmanCoding():root(nullptr){}

    ~HuffmanCoding(){
        cleanup(root);
    }

    void buildFromAVL(AVLTree&avlTree){
        std::vector<HuffmanNode*>nodes;
        collectNodes(avlTree.getRoot(),nodes);

        std::priority_queue<HuffmanNode*,std::vector<HuffmanNode*>,CompareNodes>pq;
        for(auto node:nodes){
            pq.push(node);
        }

        while(pq.size()>1){
            HuffmanNode*left=pq.top();pq.pop();
            HuffmanNode*right=pq.top();pq.pop();

            HuffmanNode*internal=new HuffmanNode("",left->frequency+right->frequency);
            internal->left=left;
            internal->right=right;
            pq.push(internal);
        }

        root=pq.top();
        generateCodes(root,"");
    }

    const std::unordered_map<std::string,std::string>&getCodes()const{
        return huffmanCodes;
    }

    void printCodes()const{
        for(const auto&pair:huffmanCodes){
            std::cout<<pair.first<<":"<<pair.second<<std::endl;
        }
    }
};
#endif
