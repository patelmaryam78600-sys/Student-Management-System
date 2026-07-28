#include <bits/stdc++.h>
using namespace std;

struct Student {
    int id;
    string name;
    string roll;
    string email;
    string course;
    string year;
    double marks; // -1 means empty
};

const string DB_FILE = "students.csv";

vector<Student> students;
int next_id = 1;

vector<string> split(const string &s, char delim) {
    vector<string> out;
    string cur;
    stringstream ss(s);
    while (getline(ss, cur, delim)) {
        out.push_back(cur);
    }
    return out;
}

string escape_field(const string &s) {
    // For simplicity we replace any '|' with '/' to avoid delimiter collision
    string out = s;
    for (char &c : out) if (c == '|') c = '/';
    return out;
}

void load_db() {
    students.clear();
    ifstream inf(DB_FILE);
    if (!inf) return;
    string line;
    bool first = true;
    while (getline(inf, line)) {
        if (line.empty()) continue;
        if (first) { first = false; continue; } // skip header
        auto cols = split(line, '|');
        Student s;
        s.id = stoi(cols.size() > 0 ? cols[0] : "0");
        s.name = cols.size() > 1 ? cols[1] : "";
        s.roll = cols.size() > 2 ? cols[2] : "";
        s.email = cols.size() > 3 ? cols[3] : "";
        s.course = cols.size() > 4 ? cols[4] : "";
        s.year = cols.size() > 5 ? cols[5] : "";
        if (cols.size() > 6 && !cols[6].empty()) s.marks = stod(cols[6]); else s.marks = -1;
        students.push_back(s);
        next_id = max(next_id, s.id + 1);
    }
}

void save_db() {
    ofstream outf(DB_FILE);
    outf << "id|name|roll|email|course|year|marks\n";
    for (auto &s : students) {
        outf << s.id << '|' << escape_field(s.name) << '|' << escape_field(s.roll) << '|' << escape_field(s.email)
             << '|' << escape_field(s.course) << '|' << escape_field(s.year) << '|' ;
        if (s.marks >= 0) outf << s.marks;
        outf << '\n';
    }
}

void list_students() {
    if (students.empty()) {
        cout << "No students found.\n";
        return;
    }
    cout << left << setw(4) << "ID" << setw(20) << "Name" << setw(12) << "Roll" << setw(20) << "Email"
         << setw(12) << "Course" << setw(8) << "Year" << setw(8) << "Marks" << '\n';
    cout << string(90, '-') << '\n';
    for (auto &s : students) {
        cout << setw(4) << s.id << setw(20) << s.name << setw(12) << s.roll << setw(20) << s.email
             << setw(12) << s.course << setw(8) << s.year;
        if (s.marks >= 0) cout << setw(8) << s.marks;
        else cout << setw(8) << "";
        cout << '\n';
    }
}

Student* find_by_roll(const string &roll) {
    for (auto &s : students) if (s.roll == roll) return &s;
    return nullptr;
}

void add_student() {
    Student s;
    s.id = next_id++;
    cout << "Enter name: "; getline(cin, s.name);
    cout << "Enter roll (unique): "; getline(cin, s.roll);
    if (s.name.empty() || s.roll.empty()) { cout << "Name and roll are required.\n"; return; }
    if (find_by_roll(s.roll)) { cout << "A student with that roll already exists.\n"; return; }
    cout << "Enter email (optional): "; getline(cin, s.email);
    cout << "Enter course (optional): "; getline(cin, s.course);
    cout << "Enter year (optional): "; getline(cin, s.year);
    cout << "Enter marks (optional - leave blank if unknown): ";
    string marks_s; getline(cin, marks_s);
    if (marks_s.empty()) s.marks = -1; else s.marks = stod(marks_s);
    students.push_back(s);
    save_db();
    cout << "Student added.\n";
}

void edit_student() {
    cout << "Enter roll of student to edit: ";
    string roll; getline(cin, roll);
    Student* s = find_by_roll(roll);
    if (!s) { cout << "Student not found.\n"; return; }
    cout << "Leave a field blank to keep current value.\n";
    cout << "Name (current: " << s->name << "): "; string tmp; getline(cin, tmp); if (!tmp.empty()) s->name = tmp;
    cout << "Roll (current: " << s->roll << "): "; getline(cin, tmp); if (!tmp.empty()) {
        if (tmp != s->roll && find_by_roll(tmp)) { cout << "Another student with that roll exists. Update cancelled.\n"; return; }
        s->roll = tmp;
    }
    cout << "Email (current: " << s->email << "): "; getline(cin, tmp); if (!tmp.empty()) s->email = tmp;
    cout << "Course (current: " << s->course << "): "; getline(cin, tmp); if (!tmp.empty()) s->course = tmp;
    cout << "Year (current: " << s->year << "): "; getline(cin, tmp); if (!tmp.empty()) s->year = tmp;
    cout << "Marks (current: "; if (s->marks >=0) cout<<s->marks; cout<<"): "; getline(cin, tmp);
    if (!tmp.empty()) s->marks = stod(tmp);
    save_db();
    cout << "Student updated.\n";
}

void delete_student() {
    cout << "Enter roll of student to delete: "; string roll; getline(cin, roll);
    for (auto it = students.begin(); it != students.end(); ++it) {
        if (it->roll == roll) {
            cout << "Delete " << it->name << " (roll " << it->roll << ")? (y/N): "; string a; getline(cin,a);
            if (!a.empty() && (a[0]=='y' || a[0]=='Y')) {
                students.erase(it);
                save_db();
                cout << "Deleted.\n";
            } else cout << "Cancelled.\n";
            return;
        }
    }
    cout << "Student not found.\n";
}

void search_students() {
    cout << "Enter search term (name, roll, email, course): "; string q; getline(cin,q);
    if (q.empty()) { cout << "Empty query.\n"; return; }
    bool found = false;
    for (auto &s : students) {
        string all = s.name + " " + s.roll + " " + s.email + " " + s.course;
        string low_all = all; string low_q = q;
        transform(low_all.begin(), low_all.end(), low_all.begin(), ::tolower);
        transform(low_q.begin(), low_q.end(), low_q.begin(), ::tolower);
        if (low_all.find(low_q) != string::npos) {
            if (!found) {
                cout << left << setw(4) << "ID" << setw(20) << "Name" << setw(12) << "Roll" << setw(20) << "Email"
                     << setw(12) << "Course" << setw(8) << "Year" << setw(8) << "Marks" << '\n';
                cout << string(90, '-') << '\n';
                found = true;
            }
            cout << setw(4) << s.id << setw(20) << s.name << setw(12) << s.roll << setw(20) << s.email
                 << setw(12) << s.course << setw(8) << s.year;
            if (s.marks >= 0) cout << setw(8) << s.marks; else cout << setw(8) << "";
            cout << '\n';
        }
    }
    if (!found) cout << "No matching students found.\n";
}

void export_csv() {
    const string out = "students_export.csv";
    ofstream of(out);
    of << "id,name,roll,email,course,year,marks\n";
    for (auto &s : students) {
        of << s.id << ',' << s.name << ',' << s.roll << ',' << s.email << ',' << s.course << ',' << s.year << ',';
        if (s.marks >= 0) of << s.marks;
        of << '\n';
    }
    cout << "Exported to " << out << "\n";
}

void show_menu() {
    cout << "\nStudent Management System (C++)\n";
    cout << "1. List students\n";
    cout << "2. Add student\n";
    cout << "3. Edit student\n";
    cout << "4. Delete student\n";
    cout << "5. Search\n";
    cout << "6. Export CSV\n";
    cout << "0. Exit\n";
    cout << "Choose an option: ";
}

int main() {
    load_db();
    while (true) {
        show_menu();
        string choice; getline(cin, choice);
        if (choice == "1") list_students();
        else if (choice == "2") add_student();
        else if (choice == "3") edit_student();
        else if (choice == "4") delete_student();
        else if (choice == "5") search_students();
        else if (choice == "6") export_csv();
        else if (choice == "0") { cout << "Goodbye!\n"; break; }
        else cout << "Invalid choice.\n";
    }
    return 0;
}
