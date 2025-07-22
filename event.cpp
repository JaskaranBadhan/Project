//EventReminder System
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <ctime>

using namespace std;

struct Event {
    int id;
    string title;
    string date; // YYYY-MM-DD
    string time; // HH:MM
    bool completed;

    string to_string() const {
        return std::to_string(id) + "|" + title + "|" + date + "|" + time + "|" + (completed ? "1" : "0");
    }

    static Event from_string(const string &line) {
        stringstream ss(line);
        string idStr, completedStr;
        Event e;
        getline(ss, idStr, '|');
        stringstream idStream(idStr);
        idStream >> e.id;
        getline(ss, e.title, '|');
        getline(ss, e.date, '|');
        getline(ss, e.time, '|');
        getline(ss, completedStr, '|');
        e.completed = (completedStr == "1");
        return e;
    }

    string getDateTime() const {
        return date + " " + time;
    }

    bool operator>(const Event& other) const {
        return getDateTime() > other.getDateTime();
    }
};

string getTodayDate() {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    stringstream ss;
    ss << (now->tm_year + 1900) << "-";
    ss << setw(2) << setfill('0') << (now->tm_mon + 1) << "-";
    ss << setw(2) << setfill('0') << now->tm_mday;
    return ss.str();
}

int daysBetween(const string& date1, const string& date2) {
    tm tm1 = {}, tm2 = {};
    sscanf(date1.c_str(), "%d-%d-%d", &tm1.tm_year, &tm1.tm_mon, &tm1.tm_mday);
    sscanf(date2.c_str(), "%d-%d-%d", &tm2.tm_year, &tm2.tm_mon, &tm2.tm_mday);
    tm1.tm_year -= 1900; tm1.tm_mon -= 1;
    tm2.tm_year -= 1900; tm2.tm_mon -= 1;
    time_t time1 = mktime(&tm1);
    time_t time2 = mktime(&tm2);
    return difftime(time2, time1) / (60 * 60 * 24);
}

void saveEvents(const vector<Event>& events) {
    ofstream file("events.txt");
    for (const auto& e : events) {
        file << e.to_string() << endl;
    }
}

int getNextId(const vector<Event>& events) {
    int maxId = 0;
    for (const auto& e : events) {
        maxId = max(maxId, e.id);
    }
    return maxId + 1;
}

vector<Event> loadEvents() {
    vector<Event> events;
    ifstream file("events.txt");
    string line;
    string today = getTodayDate();
    while (getline(file, line)) {
        if (!line.empty()) {
            Event e = Event::from_string(line);
            if (e.date < today) {
                e.completed = true; // Automatically mark expired events as completed
            }
            events.push_back(e);
        }
    }
    saveEvents(events); // Save updated completed status
    return events;
}

void addEvent() {
    vector<Event> events = loadEvents();
    Event e;
    e.id = getNextId(events);
    cout << "Enter event title: ";
    getline(cin >> ws, e.title);
    cout << "Enter event date (YYYY-MM-DD): ";
    cin >> e.date;
    cout << "Enter event time (HH:MM): ";
    cin >> e.time;
    e.completed = false;

    ofstream file("events.txt", ios::app);
    file << e.to_string() << endl;
    cout << "Event added successfully!\n";
}

void viewEvents(bool onlyPending = false, bool onlyCompleted = false) {
    auto events = loadEvents();
    priority_queue<Event, vector<Event>, greater<Event>> pq;
    string today = getTodayDate();
    for (const auto& e : events) {
        bool isExpired = e.date < today;
        if ((onlyPending && (e.completed || isExpired)) || (onlyCompleted && !e.completed)) continue;
        pq.push(e);
    }

    if (pq.empty()) {
        cout << "No matching events found.\n";
        return;
    }

    cout << "\nEvents:\n";
    while (!pq.empty()) {
        auto e = pq.top(); pq.pop();
        cout << "ID: " << e.id << " | " << e.title << " on " << e.date << " at " << e.time;
        if (e.completed)
            cout << "  [Completed]";
        else if (e.date < today)
            cout << "  [Expired]";
        else
            cout << "  [Pending]";
        cout << endl;
    }
}

void searchEventByDate() {
    string date;
    cout << "Enter date to search (YYYY-MM-DD): ";
    cin >> date;
    vector<Event> events = loadEvents();
    bool found = false;
    string today = getTodayDate();
    for (const auto& e : events) {
        if (e.date == date) {
            cout << "ID: " << e.id << " | " << e.title << " at " << e.time;
            if (e.completed)
                cout << "  [Completed]";
            else if (e.date < today)
                cout << "  [Expired]";
            else
                cout << "  [Pending]";
            cout << endl;
            found = true;
        }
    }
    if (!found) {
        cout << "No events found on this date.\n";
    }
}

void searchEventByTitle() {
    string keyword;
    cout << "Enter keyword to search in event title: ";
    getline(cin >> ws, keyword);
    vector<Event> events = loadEvents();
    bool found = false;
    string today = getTodayDate();
    for (const auto& e : events) {
        if (e.title.find(keyword) != string::npos) {
            cout << "ID: " << e.id << " | " << e.title << " on " << e.date << " at " << e.time;
            if (e.completed)
                cout << " ✅ [Completed]";
            else if (e.date < today)
                cout << " ❌ [Expired]";
            else
                cout << " ⏳ [Pending]";
            cout << endl;
            found = true;
        }
    }
    if (!found) {
        cout << "No events found matching that keyword.\n";
    }
}

void deleteEvent() {
    int id;
    cout << "Enter event ID to delete: ";
    cin >> id;
    auto events = loadEvents();
    auto it = remove_if(events.begin(), events.end(), [&](const Event& e) {
        return e.id == id;
    });
    if (it != events.end()) {
        events.erase(it, events.end());
        saveEvents(events);
        cout << "Event deleted.\n";
    } else {
        cout << "Event not found.\n";
    }
}

void showUpcomingIn7Days() {
    vector<Event> events = loadEvents();
    string today = getTodayDate();
    for (const auto& e : events) {
        if (!e.completed) {
            int days = daysBetween(today, e.date);
            if (days > 0 && days <= 7) {
                cout << "Reminder: Event '" << e.title << "' is in " << days << " day(s).\n";
            }
        }
    }
}

int main() {
    int choice;
    do {
        cout << "\n--- Event Reminder System ---\n";
        cout << "1. Add Event\n";
        cout << "2. View All Events\n";
        cout << "3. View Pending Events\n";
        cout << "4. View Completed Events\n";
        cout << "5. Search Events by Date\n";
        cout << "6. Search Events by Title\n";
        cout << "7. Delete Event\n";
        cout << "8. Exit\n";
        cout << "Choose an option: ";
        cin >> choice;

        switch (choice) {
            case 1: addEvent(); break;
            case 2: viewEvents(); break;
            case 3: viewEvents(true, false); break;
            case 4: viewEvents(false, true); break;
            case 5: searchEventByDate(); break;
            case 6: searchEventByTitle(); break;
            case 7: deleteEvent(); break;
            case 8:
                showUpcomingIn7Days();
                cout << "Goodbye!\n";
                break;
            default: cout << "Invalid option.\n";
        }
    } while (choice != 8);

    return 0;
}

