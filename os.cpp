#include<iostream>
#include<fstream>
#include<bits/stdc++.h>
using namespace std;

class pageTableEntry{
    public:
        int pid;
        int vpn;
        int pfn;
        bool valid;
        bool present;
        int lastAccess;

        pageTableEntry(int p,int v,int pf){
            pid = p; 
            vpn = v;
            pfn = pf;
            valid = true;
            present = true;
        }
        pageTableEntry(int p,int v,int pf, bool b){
            pid = p; 
            vpn = v;
            pfn = pf;
            valid = true;
            present = b;
        }
};

class TLBentry{
    public:
        int pid;
        int vpn;
        int pfn;
        bool valid;
        bool present;
        
        TLBentry(int p, int v, int pf){
            pid = p; 
            vpn = v;
            pfn = pf;
            valid = true;
            present = true;
        }
};

void handleFIFO(vector<pair<int,int> >);
void handleLRU(vector<pair<int,int> >);
void handleOPTIMAL(vector<pair<int,int> >);
bool checkRAM(pair<int,int>);
void swapPages(pair<int,int>,pair<int,int>);
void removeFromQueueTLB(pair<int,int>, queue<TLBentry*>&);
void removeFromTLB(pair<int,int>);
void printTLBQueue(queue<TLBentry*>);
void addToTLBHashMap(pair<int,int>);
void insertInTLBvector(pair<int,int>, vector<TLBentry*>&, vector<pair<int,int> >, int);
void swapRAMusingOptimal(pair<int,int>, vector<pair<int,int> >, vector<TLBentry*>&, int);
void printRAM();
void printSWAP();
void printHDD();
void printRAMQueue();
void printTLBVector(vector<TLBentry*>);
void printRAMVector();

int MAX_RAM,MAX_SWAP,MAX_HDD,pageSize,RAMsize;
int remRAMsize,remSWAPsize,remHDDsize,TLBlimit;
queue<pair<int,int> > qRAM;
vector<pair<int,int> > vectorRAM;
vector<pair<int,int> > checkValid;

map<int, vector<pageTableEntry*> > pageTables;
map<int, vector<TLBentry*> > TLB;
map<int, vector<int> > SWAP;
map<int, vector<int> > HDD;
map<int, vector<int> > RAM;

int main(){
    cout<<"Enter RAMsize, SWAPsize, HDDsize, TLBlimit, pageSize"<<endl;
  	cin>>remRAMsize>>remSWAPsize>>remHDDsize>>TLBlimit;
    cin>>pageSize;
    MAX_RAM = remRAMsize;
    MAX_HDD = remHDDsize;
    MAX_SWAP = remSWAPsize;
    remRAMsize -= remRAMsize%pageSize;
    remSWAPsize -= remSWAPsize%pageSize;
    remHDDsize -= remHDDsize%pageSize;
    RAMsize = ceil((double)MAX_RAM/(double)pageSize);
    string filename; cout<<"\nEnter file name of processes"<<endl;
    cin>>filename;
    ifstream file1(filename.c_str(),ios::in);
    if (! file1.is_open()){ cout << "Error opening file"; }
    int pid, size, numPages;
	int i = 1;
    while (! file1.eof() ){
        file1>>pid>>size;
	cout<<i<<" iteration"<<endl;
	i++;
        numPages = ceil((double)size/(double)pageSize);
        if(remRAMsize >= size){
            checkValid.push_back(make_pair(pid,size));
            vector<int> pages;
            for(int i = 0; i < numPages; i++){
                pages.push_back(i);
                qRAM.push(make_pair(pid,i));
                vectorRAM.push_back(make_pair(pid,i));
            }
            RAM.insert(make_pair(pid,pages));
            remRAMsize = remRAMsize-numPages*pageSize > 0 ? remRAMsize-numPages*pageSize:0;
            cout<<"pid "<<pid<<" allocated in RAM"<<endl;
            vector<pageTableEntry*> tempVec;
            for(int i = 0; i < numPages; i++){
                pageTableEntry* tempEntry = new pageTableEntry(pid,i,rand()%100);
                tempVec.push_back(tempEntry);
            }
            pageTables.insert(make_pair(pid,tempVec));
        }
        else if(remRAMsize + remSWAPsize >= size){
            checkValid.push_back(make_pair(pid,size));
            vector<pageTableEntry*> tempVec;
            int numPagesRAM = ceil((double)remRAMsize/(double)pageSize), numPagesSWAP = ceil(((double)size-(double)pageSize*numPagesRAM)/(double)pageSize);
            vector<int> pagesRAM;
            for(int i = 0; i < numPagesRAM; i++){
                pagesRAM.push_back(i);
                qRAM.push(make_pair(pid,i));
                vectorRAM.push_back(make_pair(pid,i));
            }
            if(numPagesRAM > 0){
                RAM.insert(make_pair(pid,pagesRAM));
                for(int i = 0; i < numPagesRAM; i++){
                    pageTableEntry* tempEntry = new pageTableEntry(pid,i,rand()%100);
                    tempVec.push_back(tempEntry);
                }
            }
            vector<int> pagesSWAP;
            for(int i = numPagesRAM; i < numPages; i++){
                pagesSWAP.push_back(i);
            }
            SWAP.insert(make_pair(pid,pagesSWAP));
            for(int i = 0; i < numPagesSWAP; i++){
                pageTableEntry* tempEntry = new pageTableEntry(pid,(numPagesRAM+i),rand()%100,false);
                tempVec.push_back(tempEntry);
            }
            pageTables.insert(make_pair(pid,tempVec));
            remRAMsize = 0;
            remSWAPsize = remSWAPsize-numPagesSWAP*pageSize > 0 ? remSWAPsize-numPagesSWAP*pageSize:0;
            cout<<"pid "<<pid<<" allocated in RAM + SWAP / SWAP"<<endl;
        }
        else if(size <= remHDDsize){ 
            cout<<"pid "<<pid<<" size "<<size<<" is too large so not added anywhere"<<endl;
            vector<int> pages;
            for(int i = 0; i < numPages; i++){
                pages.push_back(i);
            }
            HDD.insert(make_pair(pid,pages));
            remHDDsize = remHDDsize-numPages*pageSize > 0 ? remHDDsize-numPages*pageSize:0;
            cout<<"pid "<<pid<<" allocated in HDD"<<endl;
            vector<pageTableEntry*> tempVec;
            for(int i = 0; i < numPages; i++){
                pageTableEntry* tempEntry = new pageTableEntry(pid,i,rand()%100,false);
                tempVec.push_back(tempEntry);
            }
            pageTables.insert(make_pair(pid,tempVec));
        }
        else{
            cout<<"pid "<<pid<<" No more space left"<<endl;
        }
    }
    printRAM();
    printSWAP();
    printHDD();
    cout<<"remSize of RAM is : "<<remRAMsize<<" remSize of SWAP is : "<<remSWAPsize<<" remSize of HDD is : "<<remHDDsize<<endl;
    map<int,vector<pageTableEntry*> >::iterator itr1;
    for(itr1 = pageTables.begin(); itr1 != pageTables.end(); itr1++){
        vector<pageTableEntry*> temp = itr1->second;
        for(int i = 0; i < temp.size(); i++){
            cout<<"pid "<<temp[i]->pid<<" vpn "<<temp[i]->vpn<<" present bit "<<temp[i]->present<<endl;
        }
    }
    //----------------------------------------------------------------------
    // TLB manage
    string filename1; cout<<"\nEnter file name of accesses"<<endl;
    cin>>filename1;
    ifstream file2(filename1.c_str(),ios::in);
    if (! file2.is_open()){ cout << "Error opening file"; }
    int vpn, addr;
    vector<pair<int,int> > accesses;
    bool valid = true;
    bool present = false;
    while (! file2.eof() ){
        file2>>pid>>addr;
        valid = true;
        present = false;
        for(int i = 0; i < checkValid.size(); i++){
            if(pid == checkValid[i].first){
                present = true;
                if(addr >= checkValid[i].second){
                    cout<<"Access pid "<<pid<<" offset "<<addr<<" is not valid"<<endl;
                    valid = false;
                }
            }
        }
        if(valid && present){
            vpn = floor((double)addr/(double)pageSize);
            accesses.push_back(make_pair(pid,vpn));
        }
    }
    string mode;
    cout<<"\nEnter the mode from the list"<<endl;
    cout<<"fifo"<<endl;
    cout<<"opti"<<endl;
    cout<<"lru"<<endl; 
    cin>>mode;
    if(mode.compare("fifo") == 0){
        cout<<"//////////////////////////////////////////////////"<<endl;
        cout<<"FIFO mode for RAM and TLB"<<endl;
        handleFIFO(accesses);
        cout<<"//////////////////////////////////////////////////"<<endl;
    }
    else if(mode.compare("opti") == 0){
        cout<<"//////////////////////////////////////////////////"<<endl;
        cout<<"OPTIMAL mode for RAM and TLB"<<endl;
        handleOPTIMAL(accesses);
        cout<<"//////////////////////////////////////////////////"<<endl;
    }
    else if(mode.compare("lru") == 0){
        cout<<"//////////////////////////////////////////////////"<<endl;
        cout<<"LEAST RECENTLY USED mode for RAM and TLB"<<endl;
        handleLRU(accesses);
        cout<<"//////////////////////////////////////////////////"<<endl;
    }
    else{
        cout<<"invalid mode"<<endl;
    }
}

bool checkRAM(pair<int,int> access){
    vector<pageTableEntry*> vec = pageTables.find(access.first)->second;
    for(int i = 0; i < vec.size(); i++){
        if(vec[i]->vpn == access.second){
            if(vec[i]->present == true){
                return true;
            }
            else 
                break;
        }
    }
    return false;
}

void swapPages(pair<int,int> page1, pair<int,int> page2){
    vector<int>& vec = RAM.find(page1.first)->second;
    for(int i = 0; i < vec.size(); i++){
        if(vec[i] == page1.second){
            vec.erase(vec.begin()+i);
        }
    }
    if(RAM.find(page2.first) == RAM.end()){
        vector<int> tempVec;
        tempVec.push_back(page2.second);
        RAM.insert(make_pair(page2.first,tempVec));
    }
    else{
        RAM.find(page2.first)->second.push_back(page2.second);
    }

    vector<int>& vec1 = SWAP.find(page2.first)->second;
    for(int i = 0; i < vec1.size(); i++){
        if(vec1[i] == page2.second){
            vec1.erase(vec1.begin()+i);
        }
    }
    if(SWAP.find(page1.first) == SWAP.end()){
        vector<int> tempVec;
        tempVec.push_back(page1.second);
        SWAP.insert(make_pair(page1.first,tempVec));
    }
    else{
        SWAP.find(page1.first)->second.push_back(page1.second);
    }

    vector<pageTableEntry*> pageTable1 = pageTables.find(page1.first)->second;
    for(int i = 0; i < pageTable1.size(); i++){
        if(pageTable1[i]->vpn == page1.second){
            pageTable1[i]->present = false;
            break;
        }
    }

    vector<pageTableEntry*> pageTable2 = pageTables.find(page2.first)->second;
    for(int i = 0; i < pageTable2.size(); i++){
        if(pageTable2[i]->vpn == page2.second){
            pageTable2[i]->present = true;
            break;
        }
    }
}

void removeFromTLB(pair<int,int> pair1){
    if(TLB.find(pair1.first) == TLB.end()){
        return;
    }
    else{
        vector<TLBentry*>& tlbEntries = TLB.find(pair1.first)->second;
        for(int i = 0; i < tlbEntries.size(); i++){
            if(tlbEntries[i]->vpn == pair1.second){
                tlbEntries.erase(tlbEntries.begin()+i);
                break;
            }
        }
    }
}

void removeFromQueueTLB(pair<int,int> pair1, queue<TLBentry*>& qTLB){
    queue<TLBentry*> tempQ;
    while(!qTLB.empty()){
        TLBentry* tempEntry = qTLB.front();
        qTLB.pop();
        if(tempEntry->pid == pair1.first && tempEntry->vpn == pair1.second){}
        else{
            tempQ.push(tempEntry);
        }
    }
    while(!tempQ.empty()){
        TLBentry* tempEntry = tempQ.front();
        tempQ.pop();
        qTLB.push(tempEntry);
    }
}

void addToTLBHashMap(pair<int,int> pair1){
    if(TLB.find(pair1.first) == TLB.end()){
        vector<TLBentry*> tempVec;
        TLBentry* tempEntry = new TLBentry(pair1.first, pair1.second, rand()%100);
        tempVec.push_back(tempEntry);
        TLB.insert(make_pair(pair1.first,tempVec));
    }
    else{
        vector<TLBentry*>& tempVec = TLB.find(pair1.first)->second;
        TLBentry* tempEntry = new TLBentry(pair1.first, pair1.second, rand()%100);
        tempVec.push_back(tempEntry);
    }
}

void insertInTLBvector(pair<int,int> access, vector<TLBentry*>& vectorTLB, vector<pair<int,int> > accesses, int start){
    if(vectorTLB.size() < TLBlimit){
        TLBentry* temp = new TLBentry(access.first, access.second, rand()%100);
        vectorTLB.push_back(temp);
        return;
    }
    int arr[vectorTLB.size()];
    for(int i = 0; i < vectorTLB.size(); i++){
        arr[i] = INT_MAX;
    }
    for(int i = 0; i < vectorTLB.size(); i++){
        for(int j = start+1; j < accesses.size(); j++){
            if(vectorTLB[i]->pid == accesses[j].first && vectorTLB[i]->vpn == accesses[j].second){
                arr[i] = min(arr[i], j);
            }
        }
    }
    int idx = -1, max = INT_MIN;
    for(int i = 0; i < vectorTLB.size(); i++){
        if(arr[i] > max){
            idx = i;
            max = arr[i];
        }
    }
    vectorTLB.erase(vectorTLB.begin()+idx);
    TLBentry* tempEntry = new TLBentry(access.first, access.second, rand()%100);
    vectorTLB.push_back(tempEntry);
}

void swapRAMusingOptimal(pair<int,int> access, vector<pair<int,int> > accesses, vector<TLBentry*>& vectorTLB, int start){
    int arr[vectorRAM.size()];
    for(int i = 0; i < vectorRAM.size(); i++){
        arr[i] = INT_MAX;
    }
    for(int i = 0; i < vectorRAM.size(); i++){
        for(int j = start+1; j < accesses.size(); j++){
            if(vectorRAM[i].first == accesses[j].first && vectorRAM[i].second == accesses[j].second){
                arr[i] = min(arr[i], j);
            }
        }
    }
    int idx = -1, max = INT_MIN;
    for(int i = 0; i < vectorRAM.size(); i++){
        if(arr[i] > max){
            idx = i;
            max = arr[i];
        }
    }
    cout<<"Page swapped from RAM is -> pid "<<vectorRAM[idx].first<<" vpn "<<vectorRAM[idx].second<<endl;
    swapPages(vectorRAM[idx], access);
    for(int i = 0; i < vectorTLB.size(); i++){
        if(vectorTLB[i]->pid == vectorRAM[idx].first && vectorTLB[i]->vpn == vectorRAM[idx].second){
            vectorTLB.erase(vectorTLB.begin()+i);
            break;
        }
    }
    vectorRAM.erase(vectorRAM.begin()+idx);
    vectorRAM.push_back(access);
}

void printTLBQueue(queue<TLBentry*> Q){
    cout<<"TLB Queue status "<<endl;
    cout<<"---------------------------------------------------"<<endl;
    while(!Q.empty()){
        TLBentry* temp = Q.front();
        Q.pop();
        cout<<"PID "<<temp->pid<<" vpn "<<temp->vpn<<" Present "<<temp->present<<endl;
    }
    cout<<"---------------------------------------------------"<<endl;
}

void printRAM(){
    cout<<"---------------------------------------------------"<<endl;
    cout<<"Current status of RAM"<<endl;
    map<int,vector<int> >::iterator itr;
    for(itr = RAM.begin(); itr != RAM.end(); itr++){
        cout<<itr->first<<" : ";
        vector<int> temp = itr->second;
        for(int i = 0; i < temp.size(); i++){
            cout<<temp[i]<<" ";
        }
        cout<<endl;
    }
    cout<<"---------------------------------------------------"<<endl;
}

void printSWAP(){
    cout<<"---------------------------------------------------"<<endl;
    map<int,vector<int> >::iterator itr;
    cout<<"Current status of SWAP"<<endl;
    for(itr = SWAP.begin(); itr != SWAP.end(); itr++){
        cout<<itr->first<<" : ";
        vector<int> temp = itr->second;
        for(int i = 0; i < temp.size(); i++){
            cout<<temp[i]<<" ";
        }
        cout<<endl;
    }
    cout<<"---------------------------------------------------"<<endl;
}

void printHDD(){
    cout<<"---------------------------------------------------"<<endl;
    map<int,vector<int> >::iterator itr;
    cout<<"Current status of HDD"<<endl;
    for(itr = HDD.begin(); itr != HDD.end(); itr++){
        cout<<itr->first<<" : ";
        vector<int> temp = itr->second;
        for(int i = 0; i < temp.size(); i++){
            cout<<temp[i]<<" ";
        }
        cout<<endl;
    }
    cout<<"---------------------------------------------------"<<endl;
}

void printRAMQueue(){
    cout<<"---------------------------------------------------"<<endl;
    queue<pair<int,int> > temp = qRAM;
    cout<<"Queue RAM status "<<endl;
    while(!temp.empty()){
        pair<int,int> p = temp.front();
        temp.pop();
        cout<<"pid "<<p.first<<" vpn "<<p.second<<endl;
    }
    cout<<"---------------------------------------------------"<<endl;
}

void printTLBVector(vector<TLBentry*> v){
    cout<<"---------------------------------------------------"<<endl;
    cout<<"Queue TLB status "<<endl;
    for(int i = 0; i < v.size(); i++){
        cout<<"pid "<<v[i]->pid<<" vpn "<<v[i]->vpn<<endl;
    }
    cout<<"---------------------------------------------------"<<endl;
}
void printRAMVector(){
    cout<<"---------------------------------------------------"<<endl;
    cout<<"Queue RAM status "<<endl;
    for(int i = 0; i < vectorRAM.size(); i++){
        cout<<"pid "<<vectorRAM[i].first<<" vpn "<<vectorRAM[i].second<<endl;
    }
    cout<<"---------------------------------------------------"<<endl;
}

void handleFIFO(vector<pair<int,int> > accesses){
    queue<TLBentry*> qTLB;
    map<int,pair<int,int> > tlbHit_tlbMiss;
    map<int,int> pageFaults;
    for(int i = 0; i < accesses.size(); i++){
        cout<<"Status for iteration "<<i<<endl;
        cout<<"**************************************************"<<endl;
        if(pageFaults.find(accesses[i].first) == pageFaults.end()){
            pageFaults.insert(make_pair(accesses[i].first,0));
        }
        if(tlbHit_tlbMiss.find(accesses[i].first) == tlbHit_tlbMiss.end()){
            tlbHit_tlbMiss.insert(make_pair(accesses[i].first,make_pair(0,0)));
        }
        if(TLB.find(accesses[i].first) != TLB.end()){
            vector<TLBentry*> entries = TLB.find(accesses[i].first)->second;
            bool flag = false;
            for(int j = 0; j < entries.size(); j++){
                if(entries[j]->vpn == accesses[i].second){
                    tlbHit_tlbMiss.find(accesses[i].first)->second.first++;
                    flag = true;
                }
            }
            if(!flag){
                addToTLBHashMap(accesses[i]);
                tlbHit_tlbMiss.find(accesses[i].first)->second.second++;
                if(qTLB.size() == TLBlimit){
                    qTLB.pop();
                }
                TLBentry* temp = new TLBentry(accesses[i].first,accesses[i].second,rand()%100);
                qTLB.push(temp);
                if(!checkRAM(accesses[i])){
                    cout<<"TLB MISS \nPage swapped b/w RAM and SWAP"<<endl;
                    removeFromQueueTLB(qRAM.front(),qTLB);
                    removeFromTLB(qRAM.front());
                    swapPages(qRAM.front(),accesses[i]);
                    qRAM.pop();
                    qRAM.push(accesses[i]);
                    pageFaults.find(accesses[i].first)->second++;
                    printRAM();
                    printSWAP();
                }
                else{
                    cout<<"TLB HIT \nCorresponding page found in RAM"<<endl;
                }
            }
        }
        else{
            addToTLBHashMap(accesses[i]);
            tlbHit_tlbMiss.find(accesses[i].first)->second.second++;
            if(qTLB.size() == TLBlimit){
                qTLB.pop();
            }
            TLBentry* temp = new TLBentry(accesses[i].first,accesses[i].second,rand()%100);
            qTLB.push(temp);
            if(!checkRAM(accesses[i])){
                cout<<"TLB MISS \nPage swapped b/w RAM and SWAP"<<endl;
                removeFromQueueTLB(qRAM.front(),qTLB);
                removeFromTLB(qRAM.front());
                swapPages(qRAM.front(),accesses[i]);
                qRAM.pop();
                qRAM.push(accesses[i]);
                pageFaults.find(accesses[i].first)->second++;
                printRAM();
                printSWAP();
            }
            else{
                cout<<"TLB MISS \nCorresponding page found in RAM"<<endl;
            }
        }
        printTLBQueue(qTLB);
        printRAMQueue();
        cout<<"**************************************************"<<endl;
    }
    cout<<"TLB hits and misses "<<endl;
    map<int,pair<int,int> >::iterator itr;
    for(itr = tlbHit_tlbMiss.begin(); itr != tlbHit_tlbMiss.end(); itr++){
        cout<<"PID "<<itr->first<<" hits "<<itr->second.first<<" misses "<<itr->second.second<<endl;
    }
    cout<<"---------------------------------------------------"<<endl;
    cout<<"Page Faults "<<endl;
    map<int,int>::iterator itr1;
    for(itr1 = pageFaults.begin(); itr1 != pageFaults.end(); itr1++){
        cout<<"PID "<<itr1->first<<" faults "<<itr1->second<<endl;
    }
}

void handleLRU(vector<pair<int,int> > accesses){
    vector<TLBentry*> vectorTLB;  
    map<int,pair<int,int> > tlbHit_tlbMiss;
    map<int,int> pageFaults;
    bool foundInTLB = false;
    for(int i = 0; i < accesses.size(); i++){
        foundInTLB = false;
        cout<<"Status for iteration "<<i<<endl;
        cout<<"**************************************************"<<endl;
        if(pageFaults.find(accesses[i].first) == pageFaults.end()){
            pageFaults.insert(make_pair(accesses[i].first,0));
        }
        if(tlbHit_tlbMiss.find(accesses[i].first) == tlbHit_tlbMiss.end()){
            tlbHit_tlbMiss.insert(make_pair(accesses[i].first,make_pair(0,0)));
        }
        for(int j = 0; j < vectorTLB.size(); j++){
            if(vectorTLB[j]->pid == accesses[i].first && vectorTLB[j]->vpn == accesses[i].second){
                foundInTLB = true;
                cout<<"TLB HIT \nCorresponding page found in RAM"<<endl;
                tlbHit_tlbMiss.find(accesses[i].first)->second.first++;
                TLBentry* entry = vectorTLB[j];
                vectorTLB.erase(vectorTLB.begin()+j);
                vectorTLB.insert(vectorTLB.begin(),entry);
                for(int k = 0; k < RAMsize; k++){
                    if(vectorRAM[k].first == accesses[i].first && vectorRAM[k].second == accesses[i].second){
                        pair<int,int> tempEntry = vectorRAM[k];
                        vectorRAM.erase(vectorRAM.begin()+k);
                        vectorRAM.insert(vectorRAM.begin(),tempEntry);
                    }
                }
            }
        }
        if(!foundInTLB){
            tlbHit_tlbMiss.find(accesses[i].first)->second.second++;
            if(vectorTLB.size() == TLBlimit){
                vectorTLB.erase(vectorTLB.begin()+vectorTLB.size()-1);
            }
            TLBentry* tlbEntry = new TLBentry(accesses[i].first, accesses[i].second, rand()%100);
            vectorTLB.insert(vectorTLB.begin(),tlbEntry);
            if(!checkRAM(accesses[i])){
                cout<<"TLB MISS \nPage swapped b/w RAM and SWAP"<<endl;
                pageFaults.find(accesses[i].first)->second++;
                swapPages(vectorRAM[RAMsize-1], accesses[i]);
                pair<int,int> pairVectorRAM = vectorRAM[RAMsize-1];
                for(int j = 0; j < vectorTLB.size(); j++){
                    if(vectorTLB[j]->pid == pairVectorRAM.first && vectorTLB[j]->vpn == pairVectorRAM.second){
                        vectorTLB.erase(vectorTLB.begin()+j);
                    }
                }
                vectorRAM.erase(vectorRAM.end());
                vectorRAM.insert(vectorRAM.begin(), accesses[i]);
                printRAM();
                printSWAP();
            }
            else{
                cout<<"TLB MISS \nCorresponding page found in RAM"<<endl;
                for(int j = 0; j < RAMsize; j++){
                    if(vectorRAM[j].first == accesses[i].first && vectorRAM[j].second == accesses[i].second){
                        vectorRAM.erase(vectorRAM.begin()+j);
                        vectorRAM.insert(vectorRAM.begin(), accesses[i]);
                    }
                }
            }
        }
        printTLBVector(vectorTLB);
        printRAMVector();
        cout<<"**************************************************"<<endl;
    }
    cout<<"TLB hits and misses "<<endl;
    map<int,pair<int,int> >::iterator itr;
    for(itr = tlbHit_tlbMiss.begin(); itr != tlbHit_tlbMiss.end(); itr++){
        cout<<"PID "<<itr->first<<" hits "<<itr->second.first<<" misses "<<itr->second.second<<endl;
    }
    cout<<"---------------------------------------------------"<<endl;
    cout<<"Page Faults "<<endl;
    map<int,int>::iterator itr1;
    for(itr1 = pageFaults.begin(); itr1 != pageFaults.end(); itr1++){
        cout<<"PID "<<itr1->first<<" faults "<<itr1->second<<endl;
    }
}

void handleOPTIMAL(vector<pair<int,int> > accesses){
    vector<TLBentry*> vectorTLB;
    map<int,pair<int,int> > tlbHit_tlbMiss;
    map<int,int> pageFaults;
    bool foundInTLB = false;
    for(int i = 0; i < accesses.size(); i++){
        foundInTLB = false;
        cout<<"Status for iteration "<<i<<endl;
        cout<<"**************************************************"<<endl;
        if(pageFaults.find(accesses[i].first) == pageFaults.end()){
            pageFaults.insert(make_pair(accesses[i].first,0));
        }
        if(tlbHit_tlbMiss.find(accesses[i].first) == tlbHit_tlbMiss.end()){
            tlbHit_tlbMiss.insert(make_pair(accesses[i].first,make_pair(0,0)));
        }
        for(int j = 0; j < vectorTLB.size(); j++){
            if(vectorTLB[j]->pid == accesses[i].first && vectorTLB[j]->vpn == accesses[i].second){
                foundInTLB = true;
                cout<<"TLB HIT \nCorresponding page found in RAM"<<endl;
                tlbHit_tlbMiss.find(accesses[i].first)->second.first++;
            }
        }
        if(!foundInTLB){
            tlbHit_tlbMiss.find(accesses[i].first)->second.second++;
            if(!checkRAM(accesses[i])){
                cout<<"TLB MISS \nPage swapped b/w RAM and SWAP"<<endl;
                pageFaults.find(accesses[i].first)->second++;
                swapRAMusingOptimal(accesses[i], accesses, vectorTLB, i);
                insertInTLBvector(accesses[i], vectorTLB, accesses, i);
                printRAM();
                printSWAP();
            }
            else{
                insertInTLBvector(accesses[i], vectorTLB, accesses, i);
                cout<<"TLB MISS \nBut Corresponding page found in RAM"<<endl;
            }
        }
        printTLBVector(vectorTLB);
        printRAMVector();
        cout<<"**************************************************"<<endl;
    }
    cout<<"TLB hits and misses "<<endl;
    map<int,pair<int,int> >::iterator itr;
    for(itr = tlbHit_tlbMiss.begin(); itr != tlbHit_tlbMiss.end(); itr++){
        cout<<"PID "<<itr->first<<" hits "<<itr->second.first<<" misses "<<itr->second.second<<endl;
    }
    cout<<"---------------------------------------------------"<<endl;
    cout<<"Page Faults "<<endl;
    map<int,int>::iterator itr1;
    for(itr1 = pageFaults.begin(); itr1 != pageFaults.end(); itr1++){
        cout<<"PID "<<itr1->first<<" faults "<<itr1->second<<endl;
    }
}
