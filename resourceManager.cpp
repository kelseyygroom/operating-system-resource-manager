#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

struct node {
    int index;
    int units;          // this is only relevant when it comes to resource waitlist. will not be accessed in any other context
    node* next;
};


class linked_list {
    public:
        node* head;
        node* tail;
        linked_list() {
            head = nullptr;
            tail = nullptr;
        }

        ~linked_list() {
            // destructor function -- called on delete!
            // free all nodes
            node* n = head;
            node* next;
            while(n != nullptr) {
                next = n->next;
                delete n;
                n = next;
            }
            head = nullptr;
        }
        
        void insert(int n){
            // insert a node onto the tail of the linked list
            node* newNode = new node;
            newNode->index = n;
            newNode->next = nullptr;

            if (head == nullptr) {
                // first node -- is the head && the tail
                head = newNode;
                tail = newNode;
            } else {
                // otherwise, just update current tail to point
                // to new node and assign tail = newNode
                tail->next = newNode;
                tail = newNode;
            }
        }

        void insert(int n, int k){
            // diff function definition for resource waitlist.

            // insert a node onto the tail of the linked list
            node* newNode = new node;
            newNode->index = n;
            newNode->units = k;
            newNode->next = nullptr;

            if (head == nullptr) {
                // first node -- is the head && the tail
                head = newNode;
                tail = newNode;
            } else {
                // otherwise, just update current tail to point
                // to new node and assign tail = newNode
                tail->next = newNode;
                tail = newNode;
            }
        }

        int pop() {
            // remove and return the head node index
            if (head == nullptr) return -1;
            node* nodeToRemove = head;

            node* newHead = head->next;
            head = newHead;
            int index = nodeToRemove->index;
            delete nodeToRemove;
            return index;
        }

        void printList() {
            // for testing - prints out the linked list from head to tail
            node* node = head;
            while(node != nullptr) {
                printf("%d -> ", node->index);
                node = node->next;
            }
            printf("end\n");
        }

        int currentProc() {
            if (head != nullptr) { return head->index;}
            return -1;      // error otherwise
        }

        int remove(int i){
            // 1. need to search for the process with index i.
            // and remove it from the list.

            if (head == nullptr) return 0;
            if (head->index == i) {
                node* curr = head;
                head = head->next;

                if (curr == tail) {     // if ll is one node
                    tail = head;
                }
                
                delete curr;
                return 0;

            } 

            node* curr = head;
            while (curr->next != nullptr && curr->next->index != i) {
                //keep iterating
                curr = curr->next;
            }
            
            if (curr->next == nullptr) return -1;       // node not found

            //curr->next is now the node to delete. update pointers
            node* del = curr->next;
            curr->next = del->next;

            if(tail == del) {
                tail = curr;
            }

            delete del;
            return 0;
         
        }

        int get_units(int p) {
            // find process p && return num units
            node* node = head;
            while (node != nullptr) {
                if (node->index == p) return node->units;
                node = node->next;
            }
            return -1; // index not found.
        }

        int search(int p) {
            //given a process number p (index), see if it is in the ll
            node* node = head;
            while (node != nullptr) {
                if (node->index == p) return 0;
                node = node->next;
            }
            return -1;
        }
};

struct PCB {
    int state;                  // ready=1, blocked=0, free=-1. running=head of RL. 
    int parent;                 // id/index of parent process
    int priority;               // can be 0, 1, or 2
    linked_list* children;      // index of children
    linked_list* resources;     // index of resources
};

struct RCB {
    int state;                  // 1 or 0 (free or allocated)
    int inventory;              // indicate num units on resource  (0/1 have 1; 2 has 2; 3 has 3)
    linked_list* waitlist;      // ll of blocked proc indices
};

struct resource_manager {   // to store all the data for this program
    struct PCB pcb[16];
    struct RCB rcb[4];
    int free_pcb;
    linked_list* rl[3];     // RL is now a 3-level list.
};


int next_free_pcb(PCB pcb[]) {
    // iterate thru each pcb to find the next free index, if any

    for (int i = 0; i < 16; i++) {
        if (pcb[i].state == -1) {
            return i;
        }
    }
    return -1;      // no free indices!
}

int init(resource_manager* rm) {
    // PCB[0] is a running process with no parent, no children, and no resources. returns 0
    PCB* pcb = rm->pcb;
    RCB* rcb = rm->rcb;

    if (pcb[0].children != nullptr) {delete pcb[0].children;}
    if (pcb[0].resources != nullptr) {delete pcb[0].resources;}

    pcb[0].parent = -1;
    pcb[0].children = nullptr;
    pcb[0].resources = nullptr;
    pcb[0].state = 1; // state is implicit--value doesn't matter but relevant for picking next free index func
    pcb[0].priority = 0;
    
    // free all pointers!! linked lists and nodes before initialization

    // all PCB entries initialized to free except PCB[0]
    for (int i = 1; i < 16; i++) { 
        if (pcb[i].children != nullptr) {
            delete pcb[i].children;
            pcb[i].children = nullptr;
        }
        if (pcb[i].resources != nullptr) {
            delete pcb[i].resources;
            pcb[i].resources = nullptr;
        }
        pcb[i].state = -1;          // -1 = free state. other vals don't matter just make sure they r free
    }      

    // all RCB entries are initialized to free
    for(int i = 0; i < 4; i++) {
        if (i == 0) {               // to store max inventory and num units currently available
            rcb[i].inventory = 1;      
            rcb[i].state = 1;
        } else {
            rcb[i].inventory = i;
            rcb[i].state = i;
        }
        if (rcb[i].waitlist != nullptr) {
            delete rcb[i].waitlist;
            rcb[i].waitlist = nullptr;
        }     
    }
    
    // RL contains process 0
    for (int i = 0; i < 3; i++) {
        if (rm->rl[i] != nullptr) {
            delete rm->rl[i];
            rm->rl[i] = new linked_list();
        }
    }
    rm->rl[0]->insert(0);
    rm->free_pcb = 1;

    return 0;
}

int find_current_proc(resource_manager* rm) {
    // given the resource manager, locate the currently running process from the ready list.
    // note: it will always be the head of the highest priority non-empty list
    linked_list** rl = rm->rl;

    if (rl == nullptr) return -1;

    for (int i = 2; i >= 0; i--) {
        if (rl[i] != nullptr && rl[i]->head != nullptr) return rl[i]->head->index;
    }

    return -1;  // some error occured; no running process??
}

int scheduler(resource_manager* rm) {
    // find highest priority ready process j
    int proc = find_current_proc(rm);   // this will find the highest priority ready process j
    printf("process %d running\n", proc);
    return proc;
}

int create(resource_manager* rm, int priority){
    //int free_pcb, PCB pcb[], linked_list* rl
    // currently running process i creates a new child process j. returns index j

    if (priority < 0 || priority > 2) return -1;    // priority can't be anything besides 1,2,3
    // find the index of currently running process (will be parent of j)
    PCB* pcb = rm->pcb;
    linked_list** rl = rm->rl;
    int free_pcb = next_free_pcb(pcb);
    if (free_pcb == -1) return -1;      // no free pcb. error

    int curr_proc = find_current_proc(rm);
    if (curr_proc == -1) return -1;
    
    //allocate new PCB[j]
    PCB* new_pcb = &pcb[free_pcb];
    new_pcb->children = new linked_list();
    new_pcb->parent = curr_proc;
    new_pcb->resources = new linked_list();
    new_pcb->state = 1;
    new_pcb->priority = priority;

    //insert j into list of children of i
    if (pcb[curr_proc].children == nullptr) {       // check if children is initialized or not
        pcb[curr_proc].children = new linked_list();
    }

    pcb[curr_proc].children->insert(free_pcb);
    
    //insert j into RL
    rl[priority]->insert(free_pcb);

    return free_pcb;
}

int release(int r, resource_manager* rm, int proc, int k) {
    PCB* pcb = rm->pcb;
    RCB* rcb = rm->rcb;
    
    if (pcb[proc].resources == nullptr) return -1;          // if resources list is nullptr something went wrong
    if (pcb[proc].resources->search(r) == -1) return -1;    // proc doesn't hold resource r
    if (k > rcb[r].inventory) return -1;                    // releasing more units than the resource can hold
    if (k > pcb[proc].resources->get_units(r)) return -1; // releasing more units than proc holds
    

    // remove r from the resource list of process i
    if (pcb[proc].resources->remove(r) == -1) return -1;       // any error with removal is caught here


    rcb[r].state += k;                              // set state of r to avail units
    if (rcb[r].waitlist == nullptr) return r;   // waitlist empty
    node* node = rcb[r].waitlist->head;
    
    k = node->units;
    int j = node->index;
    if (rcb[r].waitlist == nullptr) return r;       // no procs waiting
    while(node != nullptr && rcb[r].state > 0) {
        node = node->next;
        if (rcb[r].state >= k) {
            rcb[r].state -= k;
            if (rcb[r].state < 0) return -1;        // rcb state should never be below 0
            pcb[j].resources->insert(r, k);
            pcb[j].state = 1;
            rcb[r].waitlist->remove(j);
            rm->rl[pcb[j].priority]->insert(j);

            // increment
            
            if (node != nullptr) {
                k = node->units; 
                j = node->index;
            }
        } else break; 
    }
    scheduler(rm);

    return r;
}

int destroy(int proc, resource_manager* rm) {
    // int proc, PCB pcb[], RCB rcb[], linked_list* rl
    // ensure that  proc is either curr running proc or child of curr proc

    // NEW APPROACH. make a list of all children && children of children etc of the process to delete. then call destroy each time
    // recursive too hard to debug unforch

    int x = 0;     // to store num procs destroyed

    PCB* pcb = rm->pcb;
    RCB* rcb = rm->rcb;

    if(pcb[proc].state == -1) return 0;    // process already destroyed
    int current_proc = find_current_proc(rm);

    if (proc != current_proc && (pcb[current_proc].children == nullptr || pcb[current_proc].children->search(proc) == -1)) return -1;     // proc is not self or child of running process. can't destory

    linked_list** rl = rm->rl;

    PCB &j = pcb[proc];
    if (j.children != nullptr) {        // there are children to destroy
        node* n = j.children->head;
        while(n != nullptr) {
            node* temp = n->next;
            x += destroy(n->index, rm);        // call destroy for all children of j
            n = temp;
        }   
    }

    // remove j from parent's list of children
    int p = j.parent;               // get parent index
    if (p != -1 && pcb[p].children != nullptr) {pcb[p].children->remove(proc);}  // remove proc j from parent's list of children

    //remove j from RL or waiting list
    // so check status of j and remove from the corresponding list
    if (j.state == 1) {
        rl[j.priority]->remove(proc);
    } else {
        // remove proc from the waitlist of any resources it's blocked on
        // remove from all RCB waitlists (not found just returns -1 && it may be in multiple waitlist)
        for (int i=0; i<4; i++) {
            if (rcb[i].waitlist != nullptr) {rcb[i].waitlist->remove(proc);}
        }
    }

    // release all resources of j
    if (j.resources != nullptr) {
        node* rsrc = j.resources->head;
        while(rsrc!=nullptr){
            int r = rsrc->index;
            int u = rsrc->units;
            rsrc = rsrc->next;
            release(r, rm, proc, u);
        }
    }
    
    // && free PCB of j !
    pcb[proc].state = -1;
    delete pcb[proc].children;
    pcb[proc].children = nullptr;
    delete pcb[proc].resources;
    pcb[proc].resources = nullptr; 
    
    return x + 1;

}

int request(int r, int k, resource_manager* rm) {
    int proc = find_current_proc(rm);

    if (r < 0 || r > 3) return -1;                  // rsrc doesn't exist
    if (proc == 0) return -1;                       // process 0 can't request resources. prevent deadlock

    RCB* rcb = rm->rcb;
    PCB* pcb = rm->pcb;
    RCB* curr_rcb = &rcb[r];
    
    PCB* curr_pcb = &pcb[proc];

    if (k > curr_rcb->inventory) return -1;         // error. requesting too many resources
    
    if (pcb[proc].resources != nullptr) {
        int units_held = pcb[proc].resources->get_units(r);
        if (units_held != -1 && k+units_held > rcb[r].inventory) return -1; // requested units + currently held units is > than all available units
        if (pcb[proc].resources->search(r) == 0) return -1;     // requesting a resource you already hold
    }
    
    //TODO: check if process already holds r or is blocked on r. - error
    if (rcb[r].waitlist != nullptr && rcb[r].waitlist->search(proc) == 0) return -1;      // proc is already blocked on resource r.

    if (curr_rcb->state >= k) {    // resource is free. units r available
        curr_rcb->state -= k;        // allocate the resources to process
        if (curr_pcb->resources == nullptr) {
            curr_pcb->resources = new linked_list();
        }
        curr_pcb->resources->insert(r, k);
        printf("resource %d allocated\n", r);
        return r;
    } else {                    // resource is blocked
        curr_pcb->state = 0;    // assign process as blocked on resource
        rm->rl[curr_pcb->priority]->pop();          // remove process from head of RL
        if (curr_rcb->waitlist == nullptr) {
            curr_rcb->waitlist = new linked_list();
        }
        curr_rcb->waitlist->insert(proc, k);      // add proc to waitlist of R
        printf("process %d blocked\n", proc);
        scheduler(rm);
        return proc;
    } 

    return -1;          // idk something went wrong if we got to this point
}

int timeout(resource_manager* rm) {
    int proc = find_current_proc(rm);           // find the current highest priority process
    rm->rl[rm->pcb[proc].priority]->pop();      // perform the context switch. pop process off head off list and insert in the end
    rm->rl[rm->pcb[proc].priority]->insert(proc);
    return scheduler(rm);                          // call scheduler which finds the new current highest priority process
}


int main() {
    // on start: create data structures PCB[16], RCB[4], RL
    // DO ERROR CHECKING LATER i.e. no command line args, wrong #, oob, etc

    std::ofstream fout("output.txt");
    

    resource_manager rm = {};
    for (int i = 0; i < 3; i++) {rm.rl[i] = new linked_list();}
    
    rm.free_pcb = 1;
    int out;                // to store function output
    int s;                  // scheduler output
    // initialize variables
    std::string in;
    std::string command;
    printf("test\n");
    while(std::getline(std::cin, in)) {       // presentation shell loop; break on eof
        // take user input

        if (in.empty()) continue;       // skip empty inputs
        command = in.substr(0, 2);          // isolate two-char command. will get args later
        
        if (command == "in") { 
            out = init(&rm);
            if (out == -1) fout << "\n-1 ";
            else {
                printf("process 0 running\n");              // edit output for submission - easy for testing rn.
                fout << "\n0 ";
            }
            fout.flush();
        } else if (command == "cr") {
            int p = stoi(in.substr(3));
            out = create(&rm, p); 
            s = scheduler(&rm);
            if (out == -1 || s == -1) fout << "-1 ";
            else fout << s << " ";
            if (out != -1)  printf("process %d created\n", out);        // edit output for submission
            else if (out == -1 || s == -1) printf("error\n");
            fout.flush();
            rm.free_pcb = next_free_pcb(rm.pcb);        // update free index in rsrc_manager for pcb.
        } else if (command == "de") {
            int proc = stoi(in.substr(3));
            out = destroy(proc, &rm);
            s = scheduler(&rm);
            rm.free_pcb = next_free_pcb(rm.pcb);        // update free index in rsrc_manager for pcb.
            if (out == -1 || s == -1) fout << "-1 ";
            else fout << s << " ";
            fout.flush();
            if (out != -1) printf("%d processes destroyed\n", out);
            else printf("error\n");  
        } else if (command == "to") {
            int proc = timeout(&rm);
            if (proc == -1) printf("error\n");
            fout << proc << " ";
            fout.flush();
        } else if (command == "rq") {
            int rsrc = stoi(in.substr(3));
            int units = stoi(in.substr(5));
            out = request(rsrc, units, &rm);
            s = scheduler(&rm);
            if (out == -1) printf("^^error w above\n");
            if (out == -1 || s == -1) fout << "-1 ";
            else fout << s << " ";
            fout.flush();
        } else if (command == "rl") {
            int rsrc = stoi(in.substr(3));
            int k = stoi(in.substr(5));
            out = release(rsrc, &rm, find_current_proc(&rm), k);
            s = scheduler(&rm);
            if (out != -1) printf("resource %d released\n", out);
            else printf("error\n");
            if (out == -1 || s == -1) fout << "-1 ";
            else fout << s << " ";
            fout.flush();
        }
    }
    
    fout.close();
    return 0;
}