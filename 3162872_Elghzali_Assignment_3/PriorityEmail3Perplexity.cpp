/*
EECS 348 Assignment 3 - CEO Email Priority Queue

Description: 
Implements an object-oriented MaxHeap priority queue in C++ to manage and prioritize
a busy CEO's incoming emails. Emails are categorized and prioritized first by sender 
tier (Boss > Subordinate > Peer > Important Person > Other Person). Ties within 
a category are broken by date (newest email read first), and further ties are 
broken by arrival sequence order (FIFO). Supports EMAIL, NEXT, READ, and COUNT 
commands processed line-by-line from a text file or standard input.
 
Input:
Command line argument pointing to a text file containing EMAIL/NEXT/READ/COUNT commands,
or standard input stream if no file argument is provided.

Output:
Terminal output for the NEXT and COUNT commands, formatted exactly to
the assignment specification.

Collaborators: None

Other Sources: Perplexity Generative AI (as required by assignment)

Author: Ismail

Creation Date: 24 September, 2026
Revision Date: 1 October, 2026

Revisions: Added prolouge and inline comments. 

*/

#include <iostream>   // Standard I/O: cout, endl
#include <fstream>    // File I/O: ifstream
#include <string>     // String class: string, substr, find, etc.
#include <cstring>    // C-string functions: sscanf for date parsing

using namespace std;

// starting size for the heap array, doubles when full
const int INITIAL_CAPACITY = 16;

class Email { // Email class holds info from each email in a priority queue
public:
    string sender;    // Sender category string (e.g., "Boss", "Peer")
    string subject;   // Email subject line
    string date;      // Date string in MM-DD-YYYY format
    int    priority;  // Numeric priority derived from sender category (5=Boss ... 1=OtherPerson)
    int    dateKey;   // Date converted to integer YYYYMMDD for comparison
    long long sequence;  // insertion order - tie-breaker if priority and date are the same

    // default constructor - just set everything to empty/zero
    Email() : sender(""), subject(""), date(""), priority(0), dateKey(0), sequence(0) {}

    static int getPriority(const string& category) {  //converts string into a quantitative priority score
        if (category == "Boss")            return 5;  // Boss is top priority
        if (category == "Subordinate")     return 4;  // next after Boss
        if (category == "Peer")            return 3;  // third
        if (category == "ImportantPerson") return 2;  // fourth
        if (category == "OtherPerson")     return 1;  // lowest priority
        return 0;  // if it's not one of the five categories, return 0
    }

    static int parseDateKey(const string& dateStr) { //Converts MM-DD-YYY into a sortable YYYYMMDD
        int month, day, year;  // pulled out of the date string
        sscanf(dateStr.c_str(), "%d-%d-%d", &month, &day, &year);  // parse the date
        return year * 10000 + month * 100 + day;  // combine into one comparable number
    }

    void setInfo(const string& cat, const string& sub, const string& dt, long long seq) { // fills information from emails fields and finds priority and date
        sender = cat;                    // save the sender category
        subject = sub;                   // save the subject
        date = dt;                       // save the date string
        priority = getPriority(cat);     // figure out the priority number
        dateKey = parseDateKey(dt);      // convert date to a number for comparison
        sequence = seq;                  // save insertion order for tie-breaking
    }

    // get methods 
    string getSender() const { return sender; }   // returns the sender category
    string getSubject() const { return subject; }  // returns the subject line
    string getDate() const { return date; }        // returns the date string
};

class MaxHeap { //MaxHeap Class is structurally a max heap
private:
    Email* data;   //array of emails
    int size;      //array of current emails in heap
    int capacity;  //how many arrays the heap can hold

    void resize() { //doubles array size when out of space
        int newCapacity = capacity * 2;  // double the capacity
        Email* newData = new (nothrow) Email[newCapacity];  // try to allocate bigger array
        if (!newData) return;  // if allocation failed, just keep the old array
        for (int i = 0; i < size; i++) {  // copy all existing emails over
            newData[i] = data[i];  // copy each email
        }
        delete[] data;           // free the old smaller array
        data = newData;           // point to the new bigger one
        capacity = newCapacity;  // update the capacity
    }

    bool hasHigherPriority(const Email& a, const Email& b) { //compares emails by priority
        if (a.priority != b.priority) {  // different categories
            return a.priority > b.priority;  // higher priority = goes first
        }
        if (a.dateKey != b.dateKey) {  // same category, different dates
            return a.dateKey > b.dateKey;  // newer date goes first
        }
        return a.sequence < b.sequence;  // same priority and date, earlier insertion goes first
    }

    void swap(Email& a, Email& b) { //swaps positions of two emails
        Email temp = a;  // temp copy of first email
        a = b;           // put second email in first spot
        b = temp;        // put saved email in second spot
    }

    void heapifyUp(int index) { //Bubbles an element upwards
        while (index > 0) {  // keep going until we hit the root
            int parent = (index - 1) / 2;  // find the parent index
            if (hasHigherPriority(data[index], data[parent])) {  // if child beats parent
                swap(data[index], data[parent]);  // swap them
                index = parent;  // move up and keep checking
            } else {
                break;  // heap property is fine, stop here
            }
        }
    }

    void heapifyDown(int index) { //pushes an element down
        while (true) {
            int left = 2 * index + 1;   // left child index
            int right = 2 * index + 2;  // right child index
            int largest = index;       // assume current is the biggest
            if (left < size && hasHigherPriority(data[left], data[largest])) //check left side for priorit
                largest = left;  //largest goes to left side
            if (right < size && hasHigherPriority(data[right], data[largest])) //check right side for priority
                largest = right; //largest goes right side
            if (largest != index) { // if the biggest isn't the current biggest
                swap(data[index], data[largest]); //swap current and largest
                index = largest; //keep going
            } else { //exit the loop if the largest is downheaped
                break;
            }
        }
    }

public:
    MaxHeap() { //constructor initializes an empty heap
        capacity = 16;              //start with 16 slots
        size = 0;                   //starts empty
        data = new Email[capacity]; //allocate the array
    }

    ~MaxHeap() { //Destroy array memory
        delete[] data;  // free the email array
    }

    void insert(const Email& email) { //adds email to heap and sorts it
        if (size >= capacity) {  // array is full, need to resize
            resize();  // double the capacity
            if (size >= capacity) return;  // resize failed, skip this insertion
        }
        data[size] = email;   // put the new email at the end
        heapifyUp(size);      // bubble it up to the right spot
        size++;               // one more email in the heap
    }

    Email* peek() { //look at highest priority email
        if (size == 0) return nullptr;  // nothing in the heap
        return &data[0];                 // root is always the highest priority
    }

    void removeTop() { //replaces root with next in line
        if (size == 0) return;  // nothing to remove
        data[0] = data[size - 1];  // move last email to the root
        size--;                     // one less email
        if (size > 0) {
            heapifyDown(0);  // fix the heap from the root down
        }
    }

    int count() { //counrs emails in heap
        return size; //return the size of the heap
    }
};

// Executive class - handles reading the file and running commands
// I renamed this from EmailManager to Executive and added currentSequence for tie-breaking
class Executive { 
private:
    MaxHeap heap;              // the heap that stores all the emails
    long long currentSequence; // tracks insertion order for tie-breaking

public:
    Executive() : currentSequence(0) {}  // start the sequence counter at 0

    void run(const string& filename) {
        ifstream file(filename);  // open the input file
        if (!file.is_open()) {  // check if the file opened successfully
            cout << "Could not open file: " << filename << endl;
            return;  // exit if the file can't be opened
        }

        string line;  // holds each line as we read it from the file

        while (getline(file, line)) {  // read one line at a time from the file
            // Remove trailing whitespace and carriage returns
            while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) { 
                line.pop_back();  // strip the last character off
            }
            if (line.empty()) continue;  // skip blank lines

            if (line.substr(0, 6) == "EMAIL ") {  // check if this is an EMAIL command
                string rest = line.substr(6);  // skip past "EMAIL " to get the fields
                size_t comma1 = rest.find(',');  // find the first comma - separates sender from subject+date
                if (comma1 == string::npos) continue;  // no comma found, skip this line
                string category = rest.substr(0, comma1);  // sender category is before the first comma
                
                size_t comma2 = rest.find_last_of(',');  // find the last comma - separates subject from date
                if (comma2 == string::npos || comma2 == comma1) continue;  // bad line, skip it
                string subject = rest.substr(comma1 + 1, comma2 - comma1 - 1);  // subject is between the two commas
                string date = rest.substr(comma2 + 1);  // date is after the last comma

                Email e;  // create a new email object
                e.setInfo(category, subject, date, currentSequence++);  // set the fields and pass the sequence number
                heap.insert(e);  // add the email to the heap

            } else if (line == "NEXT") {  // check if this is a NEXT command
                Email* e = heap.peek();  // get the top email without removing it
                if (e) {  // if there's an email to show
                    cout << "Next email:" << endl; //print next email
                    cout << "Sender: " << e->getSender() << endl;    // print the sender using getter
                    cout << "Subject: " << e->getSubject() << endl;  // print the subject using getter
                    cout << "Date: " << e->getDate() << endl;        // print the date using getter
                    cout << endl;  // blank line for formatting
                } else {  // no emails left in the heap
                    cout << "No emails to read." << endl; //prints no emails to read
                    cout << endl;  // blank line to stay consistent
                }
            } else if (line == "READ") {  // check if this is a READ command
                heap.removeTop();  // remove the top email silently, no output
            } else if (line == "COUNT") {  // check if this is a COUNT command
                cout << "There are " << heap.count() << " emails to read." << endl; //prints how many emails left to read
                cout << endl;  // blank line after the count
            }
        }
        file.close();  // close the file when done
    }
};

int main(int argc, char* argv[]) { // main function creates an Executive and runs it with the input file
    if (argc < 2) {  // check if a file was passed as argument
        cout << "Usage: " << argv[0] << " <input_file>" << endl; //outputs used input file
        return 1;  // return error if no file argument
    }

    Executive exec;      // create the executive object
    exec.run(argv[1]);   // run it with the file path

    return 0;  // all done
}