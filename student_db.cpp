
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>

using namespace std;

template <typename T>
class DynamicArray {
    static const int MIN_CAP = 4;
    T*  data;
    int count;   // number of elements currently stored
    int cap;     // allocated slots

    void resize(int newCap) {
        T* fresh = new T[newCap];
        for (int i = 0; i < count; i++) fresh[i] = data[i];
        delete[] data;
        data = fresh;
        cap = newCap;
    }

    template <typename Cmp>
    void mergeSortRec(T* tmp, int lo, int hi, Cmp less) {
        if (hi - lo < 2) return;
        int mid = lo + (hi - lo) / 2;
        mergeSortRec(tmp, lo, mid, less);
        mergeSortRec(tmp, mid, hi, less);
        int i = lo, j = mid, k = lo;
        while (i < mid && j < hi) {
            if (less(data[j], data[i])) tmp[k++] = data[j++];
            else                        tmp[k++] = data[i++];
        }
        while (i < mid) tmp[k++] = data[i++];
        while (j < hi)  tmp[k++] = data[j++];
        for (int x = lo; x < hi; x++) data[x] = tmp[x];
    }

public:
    DynamicArray() : data(new T[MIN_CAP]), count(0), cap(MIN_CAP) {}
    ~DynamicArray() { delete[] data; }

    DynamicArray(const DynamicArray& o) : data(new T[o.cap]), count(o.count), cap(o.cap) {
        for (int i = 0; i < count; i++) data[i] = o.data[i];
    }
    DynamicArray& operator=(const DynamicArray& o) {
        if (this == &o) return *this;
        delete[] data;
        cap = o.cap; count = o.count; data = new T[cap];
        for (int i = 0; i < count; i++) data[i] = o.data[i];
        return *this;
    }

    int  size()     const { return count; }
    int  capacity() const { return cap; }
    bool empty()    const { return count == 0; }

    T&       operator[](int i)       { return data[i]; }
    const T& operator[](int i) const { return data[i]; }

    // Returns false for an invalid position.
    bool insertAt(int pos, const T& v) {
        if (pos < 0 || pos > count) return false;
        if (count == cap) resize(cap * 2);
        for (int i = count; i > pos; i--) data[i] = data[i - 1];
        data[pos] = v;
        count++;
        return true;
    }

    void pushBack(const T& v) { insertAt(count, v); }

    bool removeAt(int pos) {
        if (pos < 0 || pos >= count) return false;
        for (int i = pos; i < count - 1; i++) data[i] = data[i + 1];
        count--;
        if (cap > MIN_CAP && count <= cap / 4)           // auto-shrink
            resize(cap / 2 < MIN_CAP ? MIN_CAP : cap / 2);
        return true;
    }

    void clear() {
        delete[] data;
        cap = MIN_CAP; count = 0; data = new T[cap];
    }

    template <typename Cmp>
    void sort(Cmp less) {
        if (count < 2) return;
        T* tmp = new T[count];
        mergeSortRec(tmp, 0, count, less);
        delete[] tmp;
    }
};

struct Student {
    int    roll;
    string name;
    string branch;
    double cgpa;
    Student() : roll(0), name(""), branch(""), cgpa(0.0) {}
    Student(int r, const string& n, const string& b, double c)
        : roll(r), name(n), branch(b), cgpa(c) {}
};

static bool byRoll  (const Student& a, const Student& b) { return a.roll < b.roll; }
static bool byBranch(const Student& a, const Student& b) { return a.branch < b.branch; }
static bool byCgpaDesc(const Student& a, const Student& b) { return a.cgpa > b.cgpa; }

// Case-insensitive helper
static string lower(string s) {
    for (size_t i = 0; i < s.size(); i++)
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = char(s[i] - 'A' + 'a');
    return s;
}
static bool byNameCI(const Student& a, const Student& b) { return lower(a.name) < lower(b.name); }

class StudentDB {
    DynamicArray<Student> arr;
    bool sortedByRoll;
public:
    StudentDB() : sortedByRoll(true) {}

    int size() const     { return arr.size(); }
    int capacity() const { return arr.capacity(); }
    const Student& at(int i) const { return arr[i]; }

    int linearSearch(int roll, int& comparisons) const {
        comparisons = 0;
        for (int i = 0; i < arr.size(); i++) {
            comparisons++;
            if (arr[i].roll == roll) return i;
        }
        return -1;
    }

    int binarySearch(int roll, int& comparisons) const {
        comparisons = 0;
        int lo = 0, hi = arr.size() - 1;
        while (lo <= hi) {
            int mid = lo + (hi - lo) / 2;
            comparisons++;
            if (arr[mid].roll == roll) return mid;
            if (arr[mid].roll < roll)  lo = mid + 1;
            else                       hi = mid - 1;
        }
        return -1;
    }

    int lowerBound(int roll) const {
        int lo = 0, hi = arr.size();
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (arr[mid].roll < roll) lo = mid + 1; else hi = mid;
        }
        return lo;
    }

    int insert(const Student& s) {
        int c;
        if (linearSearch(s.roll, c) != -1) return 0;
        if (sortedByRoll) arr.insertAt(lowerBound(s.roll), s);
        else              arr.pushBack(s);
        return 1;
    }

    bool remove(int roll) {
        int c;
        int idx = sortedByRoll ? binarySearch(roll, c) : linearSearch(roll, c);
        if (idx == -1) return false;
        return arr.removeAt(idx);
    }

    void ensureSortedByRoll() {
        if (!sortedByRoll) { arr.sort(byRoll); sortedByRoll = true; }
    }
    bool isSortedByRoll() const { return sortedByRoll; }

    // Sort by field: 1 roll, 2 name, 3 branch, 4 cgpa (high -> low)
    void sortBy(int field) {
        switch (field) {
            case 1: arr.sort(byRoll);     break;
            case 2: arr.sort(byNameCI);   break;
            case 3: arr.sort(byBranch);   break;
            case 4: arr.sort(byCgpaDesc); break;
        }
        sortedByRoll = (field == 1);
    }

    void clear() { arr.clear(); sortedByRoll = true; }
};

static void flushLine() { cin.ignore(numeric_limits<streamsize>::max(), '\n'); }

static int readInt(const string& prompt, int lo, int hi) {
    while (true) {
        cout << prompt;
        int v;
        if (cin >> v && v >= lo && v <= hi) { flushLine(); return v; }
        if (cin.eof()) exit(0);                       // input ended: quit cleanly
        cin.clear(); flushLine();
        cout << "  Invalid input. Enter an integer in [" << lo << ", " << hi << "].\n";
    }
}

static double readDouble(const string& prompt, double lo, double hi) {
    while (true) {
        cout << prompt;
        double v;
        if (cin >> v && v >= lo && v <= hi) { flushLine(); return v; }
        if (cin.eof()) exit(0);
        cin.clear(); flushLine();
        cout << "  Invalid input. Enter a number in [" << lo << ", " << hi << "].\n";
    }
}

static string readText(const string& prompt) {
    while (true) {
        cout << prompt;
        string s;
        if (!getline(cin, s)) exit(0);
        size_t a = s.find_first_not_of(" \t");
        if (a != string::npos) {
            size_t b = s.find_last_not_of(" \t");
            return s.substr(a, b - a + 1);
        }
        cout << "  Field cannot be empty.\n";
    }
}

static void printHeader() {
    cout << "\n" << left << setw(8) << "Roll" << setw(22) << "Name"
         << setw(10) << "Branch" << "CGPA\n" << string(46, '-') << "\n";
}
static void printRow(const Student& s) {
    cout << left << setw(8) << s.roll << setw(22) << s.name
         << setw(10) << s.branch << fixed << setprecision(2) << s.cgpa << "\n";
}
static void displayAll(const StudentDB& db) {
    if (db.size() == 0) { cout << "\n(database is empty)\n"; return; }
    printHeader();
    for (int i = 0; i < db.size(); i++) printRow(db.at(i));
    cout << "\nRecords: " << db.size() << "   Capacity: " << db.capacity() << "\n";
}

static void addStudent(StudentDB& db) {
    int    roll   = readInt("Roll number (1-999999): ", 1, 999999);
    string name   = readText("Name: ");
    string branch = readText("Branch (e.g. AIML): ");
    double cgpa   = readDouble("CGPA (0-10): ", 0.0, 10.0);
    int oldCap = db.capacity();
    if (!db.insert(Student(roll, name, branch, cgpa))) {
        cout << "  Error: roll " << roll << " already exists.\n";
        return;
    }
    cout << "  Added.";
    if (db.capacity() != oldCap)
        cout << "  [array grew: capacity " << oldCap << " -> " << db.capacity() << "]";
    cout << "\n";
}

static void deleteStudent(StudentDB& db) {
    if (db.size() == 0) { cout << "  Database is empty.\n"; return; }
    int roll = readInt("Roll number to delete: ", 1, 999999);
    int oldCap = db.capacity();
    if (!db.remove(roll)) { cout << "  Roll " << roll << " not found.\n"; return; }
    cout << "  Deleted.";
    if (db.capacity() != oldCap)
        cout << "  [array shrank: capacity " << oldCap << " -> " << db.capacity() << "]";
    cout << "\n";
}

static void searchStudent(StudentDB& db) {
    if (db.size() == 0) { cout << "  Database is empty.\n"; return; }
    int mode = readInt("Search method: 1) Linear  2) Binary : ", 1, 2);
    int roll = readInt("Roll number to find: ", 1, 999999);
    int cmp = 0, idx;
    if (mode == 2) {
        if (!db.isSortedByRoll()) {
            cout << "  (Binary search needs data sorted by roll -- sorting first.)\n";
            db.ensureSortedByRoll();
        }
        idx = db.binarySearch(roll, cmp);
    } else {
        idx = db.linearSearch(roll, cmp);
    }
    if (idx == -1) cout << "  Roll " << roll << " not found";
    else { printHeader(); printRow(db.at(idx)); cout << "\nFound at index " << idx; }
    cout << "  (" << cmp << " comparisons)\n";
}

static void sortStudents(StudentDB& db) {
    if (db.size() == 0) { cout << "  Database is empty.\n"; return; }
    int f = readInt("Sort by: 1) Roll  2) Name  3) Branch  4) CGPA (high->low) : ", 1, 4);
    db.sortBy(f);
    displayAll(db);
}

static void loadSample(StudentDB& db) {
    db.insert(Student(105, "Rahul Sen",     "AIML",  8.40));
    db.insert(Student(101, "Anika Das",     "ECE",  9.10));
    db.insert(Student(110, "Subir Ghosh",   "CSE",  7.85));
    db.insert(Student(103, "Priya Nair",    "CSE",   8.95));
    db.insert(Student(120, "Karan Mehta",   "ME",   7.20));
    db.insert(Student(108, "Zoya Khan",     "AIML",  9.40));
    cout << "  Loaded sample records.\n";
}

static void menu() {
    cout << "\n=========== Student Database (Dynamic Array) ===========\n"
         << " 1. Add student        5. Sort records\n"
         << " 2. Delete by roll     6. Display all\n"
         << " 3. Search by roll     7. Load sample data\n"
         << " 4. Clear database     0. Exit\n"
         << "=========================================================\n";
}

int main() {
    StudentDB db;
    while (true) {
        menu();
        int ch = readInt("Choice: ", 0, 7);
        switch (ch) {
            case 1: addStudent(db);    break;
            case 2: deleteStudent(db); break;
            case 3: searchStudent(db); break;
            case 4: db.clear(); cout << "  Database cleared.\n"; break;
            case 5: sortStudents(db);  break;
            case 6: displayAll(db);    break;
            case 7: loadSample(db);    break;
            case 0: cout << "Thank You!\n"; return 0;
        }
    }
}