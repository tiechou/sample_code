// Search for multiple routes by use of node removal. 
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

// Find multiple solutions via node removel. 
class Search {  
    // This vector holds the flight information.  
    vector<FlightInfo> flights; 

    // This statck is used for backtracking.  
    stack<FlightInfo> btStack; 

    // If there is a flight between from and to,  
    // store the distance of the flight in dist. 
    // Return true if the flight exists and, 
    // false otherwise. 
    bool match(string from, string to, int &dist); 

    // Given from, find any connection.  
    // Return true if a connection is found, 
    // and false otherwise. 
    bool find(string from, FlightInfo &f); 

    public:  

    // Put flights into the database.  
    void addflight(string from, string to, int dist) {  
        flights.push_back(FlightInfo(from, to, dist));  
    }  

    // Show the route and total distance.  
    void route();  

    // Determine if there is a route between from and to.   
    void findroute(string from, string to); 

    // Return true if a route has been found. 
    bool routefound() { 
        return btStack.size() != 0; 
    } 

    // Return flight on top of stack. 
    FlightInfo getTOS() { 
        return btStack.top(); 
    } 

    // Reset all skip fields.  
    void resetAllSkip(); 

    // Remove a connection.  
    void remove(FlightInfo f); 
};  

// Show the route and total distance.  
void Search::route()  
{  
    stack<FlightInfo> rev;  
    int dist = 0;  
    FlightInfo f;  

    // Reverse the stack to display route.  
    while(!btStack.empty()) { 
        f = btStack.top(); 
        rev.push(f); 
        btStack.pop(); 
    } 

    // Display the route. 
    while(!rev.empty()) { 
        f = rev.top(); 
        rev.pop();  
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
bool Search::match(string from, string to, int &dist)  
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

// Given from, find any connection.  
// Return true if a connection is found, 
// and false otherwise. 
bool Search::find(string from, FlightInfo &f)  
{  
    for(unsigned i=0; i < flights.size(); i++) {  
        if(flights[i].from == from && !flights[i].skip) {  
            f = flights[i]; 
            flights[i].skip = true; // prevent reuse  

            return true;  
        }  
    }  

    return false;  
}  

// Determine if there is a route between from and to.   
void Search::findroute(string from, string to)  
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

// Reset all skip fields.  
void Search::resetAllSkip() {  
    for(unsigned i=0; i< flights.size(); i++)  
        flights[i].skip = false;  
}  

// Remove a connection.  
void Search::remove(FlightInfo f) {  
    for(unsigned i=0; i< flights.size(); i++)  
        if(flights[i].from == f.from &&  
                flights[i].to == f.to)  
            flights[i].from = "";  
}  


// Node removal version. 
int main() {  
    char to[40], from[40]; 
    Search ob; 
    FlightInfo f; 

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

        // If no new route was found, then end. 
        if(!ob.routefound()) break; 

        // Save the flight on top-of-stack. 
        f = ob.getTOS();  

        ob.route(); // display the current route. 

        ob.resetAllSkip(); // reset the skip fields 

        // Remove last flight in previous solution 
        // from the flight database. 
        ob.remove(f); 
    } 

    return 0; 
}  

