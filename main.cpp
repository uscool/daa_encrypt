#include<iostream>
#include<fstream>
#include<sstream>
#include<unordered_map>
#include"avl_tree.hpp"
#include"huffman.hpp"

using namespace std;

string tolc(const string&s){
    string r=s;
    for(char&c:r){
        c=tolower(c);
    }
    return r;
}

string clnWrd(const string&w){
    string r;
    for(char c:w){
        if(isalnum(c)){
            r+=c;
        }
    }
    return r;
}

int main(){
    string f;
    cout<<"Enter the input file name:";
    cin>>f;

    ifstream i(f);
    if(!i.is_open()){
        cerr<<"Error opening file:"<<f<<endl;
        return 1;
    }

    unordered_map<string,int>wf;
    string l,w;

    while(getline(i,l)){
        istringstream is(l);
        while(is>>w){
            w=clnWrd(tolc(w));
            if(!w.empty()){
                wf[w]++;
            }
        }
    }
    i.close();

    AVLTree at;
    for(const auto&p:wf){
        at.insert(p.first,p.second);
    }

    cout<<"\nWord frequencies in AVL tree (inorder traversal):\n";
    at.printInorder();

    HuffmanCoding h;
    h.buildFromAVL(at);

    cout<<"\nHuffman codes for each word:\n";
    h.printCodes();

    const auto&hc=h.getCodes();

    return 0;
}
