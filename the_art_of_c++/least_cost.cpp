// Find an "optimal" solution using least-cost with path removal. 
#include <iostream> 
#include <stack> 
#include <string> 
#include <vector> 

using namespace std; 

// Flight information.  
struct FlightInfo {  
    string from;  // departure city 
    string to;    // destination city 
    int distance; // distance between from and to 
    bool skip;    // used in backtracking  

    FlightInfo() { 
        from = ""; 
        to = ""; 
        distance = 0; 
        skip = false; 
    } 

    FlightInfo(string f, string t, int d) {  
        from = f;  
        to = t;  
        distance = d;  
        skip = false;  
    }  
};  

const int MAXDIST = 100000; 

// Find connections using least cost. 
class Optimal {  
    // This vector holds the flight information.  
    vector<FlightInfo> flights; 

    // This statck is used for backtracking.  
    stack<FlightInfo> btStack;  

    // This stack holds the optimal soltuion. 
    stack<FlightInfo> optimal;  

    int minDist; 

    // If there is a flight between from and to,  
    // store the distance of the flight in dist. 
    // Return true if the flight exists and, 
    // false otherwise. 
    bool match(string from, string to, int &dist); 

    // Least-cost version. 
    // Given from, find the closest connection.  
    // Return true if a connection is found, 
    // and false otherwise. 
    bool find(string from, FlightInfo &f); 

    public:  

    // Constructor 
    Optimal() { 
        minDist = MAXDIST; 
    } 

    // Put flights into the database.  
    void addflight(string from, string to, int dist) {  
        flights.push_back(FlightInfo(from, to, dist));  
    } 

    // Show the route and total distance.  
    void route();  

    // Display the optimal route. 
    void Optimal::showOpt(); 

    // Determine if there is a route between from and to.   
    void findroute(string from, string to); 

    // Return true if a route has been found. 
    bool routefound() { 
        return btStack.size() != 0; 
    } 
};  

// Show the route and total distance.  
void Optimal::route()  
{  
    stack<FlightInfo> optTemp;  
    int dist = 0;  
    FlightInfo f;  

    // Reverse the stack to display route.  
    while(!btStack.empty()) { 
        f = btStack.top(); 
        optTemp.push(f); 
        btStack.pop(); 
        dist += f.distance; 
    } 

    // If shorter, keep this route. 
    if(minDist > dist) { 
        optimal = optTemp; 
        minDist = dist; 
    } 
} 

// Display the optimal route. 
void Optimal::showOpt() 
{ 
    FlightInfo f; 
    int dist = 0; 

    cout <<"Optimal solution is:\n";  

    // Display the optimal route. 
    while(!optimal.empty()) { 
        f = optimal.top(); 
        optimal.pop();  
        cout << f.from << " to ";  
        dist += f.distance;  
    }  

    cout << f.to << endl;  
    cout << "Distance is " << dist << endl;  
}  

// If there is a flight between from and to,  
// store the distance of the flight in dist. 
// Return true if the flight exists and, 
// false otherwise. 
bool Optimal::match(string from, string to, int &dist)  
{  
    for(unsigned i=0; i < flights.size(); i++) {  
        if(flights[i].from == from &&  
                flights[i].to == to && !flights[i].skip)  
        {  
            flights[i].skip = true; // prevent reuse  
            dist = flights[i].distance; 
            return true; 
        }  
    }  

    return false; // not found   
}  

// Least-cost version. 
// Given from, find the closest connection.  
// Return true if a connection is found, 
// and false otherwise. 
bool Optimal::find(string from, FlightInfo &f)  
{  
    int pos = -1; 
    int dist = MAXDIST; // longer than longest flight 

    for(unsigned i=0; i < flights.size(); i++) {  
        if(flights[i].from == from && !flights[i].skip)  
        {  
            // Use the shortest flight. 
            if(flights[i].distance < dist) { 
                pos = i; 
                dist = flights[i].distance; 
            } 
        } 
    } 

    if(pos != -1) { 
        f = flights[pos]; 
        flights[pos].skip = true; // prevent reuse  

        return true;  
    }  

    return false;  
}  

// Determine if there is a route between from and to.   
void Optimal::findroute(string from, string to)  
{  
    int dist;  
    FlightInfo f;  

    // See if at destination.  
    if(match(from, to, dist)) { 
        btStack.push(FlightInfo(from, to, dist));  
        return;  
    }  

    // Try another connection.  
    if(find(from, f)) { 
        btStack.push(FlightInfo(from, to, f.distance));  
        findroute(f.to, to);  
    }  
    else if(!btStack.empty()) {  
        // Backtrack and try another connection.  
        f = btStack.top(); 
        btStack.pop();  
        findroute(f.from, f.to);  
    }  
} 

// Find "optimal" solution by using least-cost with path removal. 
int main() {  
    char to[40], from[40]; 
    Optimal ob; 

    // Add flight connections to database. 
    ob.addflight("New York", "Chicago", 900);  
    ob.addflight("Chicago", "Denver", 1000);  
    ob.addflight("New York", "Toronto", 500);  
    ob.addflight("New York", "Denver", 1800);  
    ob.addflight("Toronto", "Calgary", 1700);  
    ob.addflight("Toronto", "Los Angeles", 2500);  
    ob.addflight("Toronto", "Chicago", 500);  
    ob.addflight("Denver", "Urbana", 1000);  
    ob.addflight("Denver", "Houston", 1000);  
    ob.addflight("Houston", "Los Angeles", 1500);  
    ob.addflight("Denver", "Los Angeles", 1000);  

    // Get departure and destination cities.  
    cout << "From? ";  

    cin.getline(from, 40); 
    cout << "To? ";  

    cin.getline(to, 40); 

    // Find multiple solutions. 
    for(;;) { 
        // See if there is a connection. 
        ob.findroute(from, to);  

        // If no route found, then end. 
        if(!ob.routefound()) break; 

        ob.route();  
    } 

    // Display optimal solution.  
    ob.showOpt();  

    return 0; 

} 

