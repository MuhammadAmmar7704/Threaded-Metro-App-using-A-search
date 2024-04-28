#pragma once
#include "Data Structures.h"
#include <thread>
#include <mutex>
class Travel {
public:

	Queue <Pair<int, vector<int>>> route;

    sf::Text from_text;



    int findMinimumVertex(vector<int>& distance, bool*& vis, const int& n) {
        int ans = -1;

        for (int i = 0; i < n; ++i) {
            if (!vis[i] && (ans == -1 || distance[i] < distance[ans])) {
                ans = i;
            }
        }
        return ans;
    }

    float distanceToDestination(int source, int destination, vector<string>& stations, mutex& mtx, DATA& d) {
        mtx.lock();
        graph G;
        stationDATA s1 = d.getstation(stations[source]);
        stationDATA s2 = d.getstation(stations[destination]);
        float t = graph::distanceBetweenPoints(s1.relative_position, s2.relative_position);
        mtx.unlock();
        return t;
    }

    bool isValid(int row, int col, int rowSize, int colSize, mutex& mtx) {
        mtx.lock();
        bool valid = (row >= 0 && col >= 0 && row < rowSize&& col < colSize);
        mtx.unlock();
        return valid;
    }
    struct ThreadData {
        int station;                             // Current station being explored
        vector<vector<int>>& graph;              // Reference to the graph
        const int& n;                            // Number of stations
        int destination;                         // Destination station
        vector<string>& stations;                // List of stations
        vector<int>& distance;                   // Stores distance from source to each station
        vector<int>& parent;                     // Stores parent station for path reconstruction
        bool* visited;                           // Keeps track of visited stations
        mutex& mtx;
    };
    static void* exploreStation(void* args) {
        ThreadData* data = (ThreadData*)(args);

        for (int neighbor = 0; neighbor < data->n; ++neighbor) {
            if (data->graph[data->station][neighbor] != 0 && !data->visited[neighbor]) {
                float tentativeDistance = data->distance[data->station] + data->graph[data->station][neighbor];

                data->mtx.lock();

                if (tentativeDistance < data->distance[neighbor]) {
                    data->distance[neighbor] = tentativeDistance;
                    data->parent[neighbor] = data->station;
                }

                data->mtx.unlock();
            }
        }

        return NULL; // Indicate thread completion (optional)
    }

    Pair<int, vector<int>> astarSearchThreaded(vector<vector<int>>& graph, const int& n, int source, int destination, vector<string>& stations, DATA& d) {
        vector<int> distance(n, INT_MAX);
        vector<int> parent(n, -1);
        bool* visited = new bool[n];

        for (int i = 0; i < n; ++i) {
            distance[i] = INT_MAX;
            visited[i] = false;
        }

        distance[source] = 0;

        mutex mtx;
        for (int i = 0; i < n - 1; ++i) {
            int minStation = -1;
            float minFscore = (float)INT_MAX;

            for (int v = 0; v < n; ++v) {
                if (!visited[v] && distance[v] != INT_MAX) {
                    float fscore = distance[v] + distanceToDestination(v, destination, stations, mtx, d);
                    if (fscore < minFscore) {
                        minFscore = fscore;
                        minStation = v;
                    }
                }
            }

            if (minStation == -1) {
                break;
            }

            visited[minStation] = true;


            vector<thread> threads;
            //vector<pthread_t> pthreads(n); // Array to store pthread handles

            for (int neighbor = 0; neighbor < n; ++neighbor) {
                if (graph[minStation][neighbor] != 0 && !visited[neighbor]) {
                    //setdata(data, minStation, graph, n, destination, stations, distance, parent, visited);
                    ThreadData* data = new ThreadData{ minStation, graph, n, destination, stations, distance, parent, visited, mtx };
                    //pthread_create(&pthreads[neighbor], NULL, exploreStation, &data);
                    /*if (status != 0) {
                        cerr << "Error Thread Creation failed\n";
                        goto here;
                    }*/
                    /*if (!data) {
                        cerr << "Error: Memory alocation failed\n";
                        goto here;
                    }*/
                    //pthread_create(&pthreads[neighbor], NULL, exploreStation, data);
                    //if ((&pthreads[neighbor], NULL, exploreStation, data)) {
                        //cerr << "Error creating thread: " << endl;
                        // Handle thread creation error (optional)
                        //goto here;
                    //}
                    threads.push_back(thread(thread(&exploreStation, data))); // Keep a thread object for joining
                }
            }

            // Wait for all threads to finish
            for (thread& thread : threads) {
                thread.join();
            }

            // Alternatively, use pthread_join with the pthreads array
            /*for (pthread_t& pthread : pthreads) {
                pthread_join(pthread, NULL);
            }*/

            //free(pthreads.data()); // Deallocate pthreads array memory (optional)
        }

        if (distance[destination] == INT_MAX) {
        here:
            delete[] visited; // Deallocate memory before returning
            return make_Pair(INT_MAX, vector<int>()); // No path found
        }

        vector<int> shortestPath;
        int current = destination;
        while (current != -1) {
            shortestPath.push_back(current);
            current = parent[current];
        }
        reverse(shortestPath.begin(), shortestPath.end()); // Correct order from source to destination

        delete[] visited; // Deallocate memory
        return make_Pair(distance[destination], shortestPath);
    }


   /* Pair<int, vector<int>> dijkstra(vector<vector<int>>& graph, const int& n, int source, int destination) {
        vector<int> distance(n, INT_MAX);
        vector<int> parent(n, -1);
        bool* vis = new bool[n];

        for (int i = 0; i < n; i++) {
            vis[i] = false;
        }

        distance[source] = 0;
        for (int i = 0; i < n - 1; ++i) {
            int u = findMinimumVertex(distance, vis, n);
                
            vis[u] = true;

            for (int v = 0; v < n; ++v) {
                if (graph[u][v] && !vis[v] && distance[u] != INT_MAX &&
                    distance[u] + graph[u][v] < distance[v]) {
                    distance[v] = distance[u] + graph[u][v];
                    parent[v] = u;
                }
            }
        }

        if (distance[destination] == INT_MAX) {
            return make_Pair(INT_MAX, vector<int>());
        }

        vector<int> shortestPath;
        int current = destination;
        while (current != -1) {
            shortestPath.push_back(current);
            current = parent[current];
        }
        reverse(shortestPath.begin(), shortestPath.end());

        return make_Pair(distance[destination], shortestPath);
    }*/

    template <typename K, typename V>
    K getKeyByValue(const unordered_map<K, V>& map, const V& value) {
        for (const auto& Pair : map) {
            if (Pair.second == value) {
                return Pair.first; 
            }
        }
        return K();
    }

    void clear() {
        while (!route.empty()) {
            route.pop();
        }
    }

    bool addstop(sf::RenderWindow& w, TrieNode* r, graph& g, DATA& d) {
        static string endprev; 
        vector<string> a;
        if (route.empty()) {
            string start = Inputfunc(w, r, "Add starting point");
            if (g.doesExist(start)) {
                cout << "STATION NAME NOT FOUND " << endl;
                return false;
            }
            string end = Inputfunc(w, r, "Add destination point");
            if (g.doesExist(end)) {
                cout << "STATION NAME NOT FOUND " << endl;
                return false;
            }

            Pair<int, vector<int>> result = 
            astarSearchThreaded(g.adjacency_matrix, g.stations, g.nameofindex[start], g.nameofindex[end], g.nameofstations, d);

            if (result.first == INT_MAX) {
                cout << "No PATH EXISTS " << endl;
                return false;
            }

            route.push(result);
            endprev = end;
        }
        else {
            string end = Inputfunc(w, r, "Add stop point");
            if (g.doesExist(end)) {
                cout << "STATION NAME NOT FOUND " << endl;
                return false;
            }
            string start = endprev;

            Pair<int, vector<int>> result =
                astarSearchThreaded(g.adjacency_matrix, g.stations, g.nameofindex[start], g.nameofindex[end], g.nameofstations, d);

            if (result.first == INT_MAX) {
                cout << "No PATH EXISTS " << endl;
                return false;
            }


            route.push(result);
            endprev = end;
        }
        return true;
    }

    void removestop(string name, graph& g) {
        bool found = 0;
        queue <Pair<int, vector<int>>> temp;
        while (!route.empty()) {

            int shortestDistance = route.front().first;
            vector<int> shortestPath = route.front().second;
            if (getKeyByValue(g.nameofindex, shortestPath[0]) == name) {
                while (!route.empty()){
                    route.pop();
                    found = 1;
                }
                break;
            }
            temp.push(route.front());
            route.pop();
        }
        
        while (!temp.empty()) {
            route.push(temp.front());
            temp.pop();
        }
        /*if (!found) {
            sf::RenderWindow w(sf::VideoMode(800, 600), "SFML Text Example");
            sf::Text totaldis;
            sf::Font font;
            if (!font.loadFromFile("Winter Night.ttf")) {
                return;
            }
            while (w.isOpen()) {
                sf::Event e;
                while (w.pollEvent(e)) {

                    if (e.type == sf::Event::Closed) {
                        w.close();
                    }
                    else if (e.type == sf::Event::KeyPressed) {
                        if (e.key.code == sf::Keyboard::Enter) {
                            return;
                        }
                    }

                }
                int pos_y = 50;
                setlabel(totaldis, sf::Vector2f(50, pos_y),
                    "No Vertex Found " , font, 15);
                w.draw(totaldis);
                w.display();
                w.clear();
            }
        }*/
    }
    
    void printroute(graph& g, sf::RenderWindow& w, sf::Font& font,
        int pos_y = 350, bool setorprint = 1, bool mainscreen = 0) {

        static string dis;
        static string path;
        
        if (route.empty()) {
            return;
        }
        Pair<int, vector<int>> result;
        if(!mainscreen)
            result = route.front();
        else
            result = route.back();

        int shortestDistance = result.first;
        vector<int> shortestPath = result.second;


        sf::Text temp;
        if (shortestDistance == INT_MAX) {
            cout << "No path found from source to destination." << endl;
        }
        
        else {
            if (setorprint) {
                setlabel(temp, sf::Vector2f(50, pos_y),dis, font, 15);
                w.draw(temp);
                pos_y += 20;
                setlabel(temp, sf::Vector2f(50, pos_y), path, font, 15);
                w.draw(temp);


            }
            else {

                string start = getKeyByValue(g.nameofindex, shortestPath[0]);
                string end = getKeyByValue(g.nameofindex, shortestPath[shortestPath.size() - 1]);

                dis = "Shortest distance: " + to_string(shortestDistance);
                path = "Route from " + start + " to " + end + " = ";

                for (int i = 0; i < shortestPath.size(); ++i) {
                    string a = getKeyByValue(g.nameofindex, shortestPath[i]);
                    path += a;
                    if (i == 5) {
                        path += "\n";
                    }
                    if(i != shortestPath.size() - 1)
                        path += " -> ";
                }
            }
        }

    }



    void routeScreen(graph& g, sf::Font& f, sf::RenderWindow& w) {
        queue <Pair<int, vector<int>>> routetemp;
        int distance = 0;
        sf::Text totaldis;

        while (!route.empty()) {
            routetemp.push(route.front());
            distance += route.front().first;
            route.pop();
        }
        while (!routetemp.empty()) {
            route.push(routetemp.front());
            routetemp.pop();
        }

        while (w.isOpen()) {
            sf::Event e;

            while (w.pollEvent(e)){

                if (e.type == sf::Event::Closed) {
                    w.close();
                }
                else if (e.type == sf::Event::KeyPressed) {
                    if (e.key.code == sf::Keyboard::Enter) {
                        return;
                    }
                }

            }
            int pos_y = 50;
            while (!route.empty()) {
                routetemp.push(route.front());
                printroute(g, w, f, pos_y, false);
                printroute(g, w, f, pos_y);
                
                route.pop();
                pos_y += 60;
            }
            while (!routetemp.empty()) {
                route.push(routetemp.front());
                routetemp.pop();
            }
            setlabel(totaldis, sf::Vector2f(50, pos_y),
                "TOTAL DISTANCE = " + to_string(distance), f, 15);
            w.draw(totaldis);
            setlabel(totaldis, sf::Vector2f(50, pos_y + 40),
                "TOTAL FARE = " + to_string(distance * 15) + " Rupees", f, 15);
            w.draw(totaldis);
            w.display();
            w.clear();
        }
    }


};