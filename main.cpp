#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <queue>
#include <stack>
#include <iomanip>

using namespace std;

const int INF = 10000000;
const int PARAM = 3;

struct MyGraph {
    vector<vector<int>> adjMatrix;
    vector<vector<int>> distMatrix;
    vector<vector<int>> weightMatrix;
    vector<vector<int>> costMatrix;
    vector<vector<int>> capacityMatrix;
    vector<vector<int>> flowMatrix;
    int verticesCount;
    bool isGenerated;
    bool isDirected;
    int lastMaxFlow;
    int lastSource;
    int lastSink;

    MyGraph() : verticesCount(0), isGenerated(false), isDirected(false),
                lastMaxFlow(0), lastSource(-1), lastSink(-1) {}

    void initMatrices(int n, bool directed = false) {
        verticesCount = n;
        isDirected = directed;
        adjMatrix.assign(n, vector<int>(n, 0));
        distMatrix.assign(n, vector<int>(n, INF));
        weightMatrix.assign(n, vector<int>(n, 0));
        costMatrix.assign(n, vector<int>(n, 0));
        capacityMatrix.assign(n, vector<int>(n, 0));
        flowMatrix.assign(n, vector<int>(n, 0));
        lastMaxFlow = 0;
        lastSource = -1;
        lastSink = -1;
        isGenerated = true;
    }

    void reset() {
        verticesCount = 0;
        isGenerated = false;
        isDirected = false;
        adjMatrix.clear();
        distMatrix.clear();
        weightMatrix.clear();
        costMatrix.clear();
        capacityMatrix.clear();
        flowMatrix.clear();
        lastMaxFlow = 0;
        lastSource = -1;
        lastSink = -1;
    }
};

class RandomGenerator {
private:
    mt19937 gen;
    normal_distribution<> normalDistrubution;
    uniform_int_distribution<int> boolDist;

public:
    RandomGenerator() : gen(random_device{}()), normalDistrubution(0.0, 1), boolDist(0, 1) {}

    double getNormal() {
        return normalDistrubution(gen);
    }

    bool getBool() {
        return boolDist(gen);
    }
};

double PirsonDistribution(int n, RandomGenerator& rng) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        double u = rng.getNormal();
        sum += u * u;
    }
    return sum;
}

// возведение матрицы в степень
vector<vector<int>> matrixPower(const vector<vector<int>>& matrix, int power) {
    int n = matrix.size();
    if (n == 0) return vector<vector<int>>();
    if (power == 1) return matrix;
    
    vector<vector<int>> result = matrix;
    
    for (int p = 1; p < power; p++) {
        vector<vector<int>> next(n, vector<int>(n, 0));
        
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                for (int k = 0; k < n; k++) {
                    if (result[i][k] && matrix[k][j]) {
                        next[i][j] += result[i][k] * matrix[k][j];
                    }
                }
            }
        }
        
        result = next;
    }
    
    return result;
}

// вспомогательная функция для вывода матрицы
void printMatrix(const vector<vector<int>>& matrix, const string& name) {
    int n = matrix.size();
    cout << "\n" << name << ":\n";

    int colW = 4;
    int labelW = 3;

    cout << string(labelW, ' ');
    for (int j = 0; j < n; j++) {
        cout << setw(colW) << j;
    }
    cout << "\n" << string(labelW, ' ') << string(n * colW, '-') << "\n";

    for (int i = 0; i < n; i++) {
        cout << setw(labelW - 1) << i << "|";
        for (int j = 0; j < n; j++) {
            cout << setw(colW) << matrix[i][j];
        }
        cout << "\n";
    }
}

// проверка связности через BFS от вершины 0
bool isConnected(const vector<vector<int>>& adjMatrix) {
    int n = adjMatrix.size();
    if (n <= 1) return true;

    vector<bool> visited(n, false);
    queue<int> q;
    q.push(0);
    visited[0] = true;
    int count = 1;

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v = 0; v < n; v++) {
            if (adjMatrix[u][v] == 1 && !visited[v]) {
                visited[v] = true;
                q.push(v);
                count++;
            }
        }
    }

    return count == n;
}

// подсчет количества ребер в графе
int countEdges(const vector<vector<int>>& adjMatrix, bool directed) {
    int n = adjMatrix.size();
    if (n == 0) return 0;
    
    int edges = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (adjMatrix[i][j] == 1) {
                edges++;
            }
        }
    }
    
    if (!directed) {
        edges /= 2;
    }
    
    return edges;
}

// подсчет реальных степеней вершин по построенной матрице
vector<int> computeActualDegrees(const vector<vector<int>>& adjMatrix, bool directed) {
    int n = adjMatrix.size();
    if (n == 0) return vector<int>();
    
    vector<int> degrees(n, 0);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (adjMatrix[i][j] == 1) {
                degrees[i]++;
            }
        }
    }
    
    return degrees;
}

// генерация степеней по распределению пирсона
vector<int> generateDegrees(int n, RandomGenerator& rng, bool directed) {
    if (n == 0) return vector<int>();
    if (n == 1) {
        vector<int> degrees(1, 0);
        return degrees;
    }
    
    vector<int> degrees(n);
    
    for (int i = 0; i < n; i++) {
        double x = PirsonDistribution(PARAM, rng);
        degrees[i] = (int)round(x) + 1;
        degrees[i] = max(1, min(degrees[i], n - 1));
    }
    
    if (!directed) {
        int sumDeg = accumulate(degrees.begin(), degrees.end(), 0);
        if (sumDeg % 2 != 0) {
            degrees[0] = min(degrees[0] + 1, n - 1);
        }
    }
    
    return degrees;
}

bool buildDirected(const vector<int>& outDegrees, vector<vector<int>>& adjMatrix, RandomGenerator& rng) {
    int n = outDegrees.size();
    if (n == 0) return true;
    if (n == 1) {
        if (outDegrees[0] == 0) {
            adjMatrix[0][0] = 0;
            return true;
        }
        return false;
    }
    
    for (int i = 0; i < n; i++) {
        fill(adjMatrix[i].begin(), adjMatrix[i].end(), 0);
    }
    
    vector<int> remainingOut = outDegrees;
    
    for (int i = 0; i < n; i++) {
        int need = remainingOut[i];
        if (need == 0) continue;
        
        vector<int> availablePositions;
        for (int j = i + 1; j < n; j++) {
            availablePositions.push_back(j);
        }
        
        while (need > 0 && !availablePositions.empty()) {

            int randomIndex = (static_cast<int>(round(PirsonDistribution(5, rng)))) % availablePositions.size();
            int position = availablePositions[randomIndex];
            
            adjMatrix[i][position] = 1;
            
            availablePositions[randomIndex] = availablePositions.back();
            availablePositions.pop_back();
            
            need--;
        }
        
    }
    
    return true;
}

bool buildUndirected(const vector<int>& degrees, vector<vector<int>>& adjMatrix, RandomGenerator& rng) {
    int n = degrees.size();
    if (n == 0) return true;
    if (n == 1) {
        if (degrees[0] == 0) {
            adjMatrix[0][0] = 0;
            return true;
        }
        return false;
    }
    
    buildDirected(degrees, adjMatrix, rng);

    
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (adjMatrix[i][j] == 1) {
                adjMatrix[j][i] = 1;
            }
        }
    }
    
    return true;
}

void generateGraph(MyGraph& graph, RandomGenerator& rng) {
    int n;
    bool directed;
    
    cout << "введите количество вершин: ";
    cin >> n;
    
    if (n == 0) {
        cout << "пустой граф\n";
        graph.initMatrices(n, false);
        return;
    }
    
    cout << "тип графа (0 - неориентированный, 1 - ориентированный): ";
    cin >> directed;
    
    graph.initMatrices(n, directed);
    
    if (n == 1) {
        cout << "\nграф с одной вершиной успешно сгенерирован\n";
        cout << "степень вершины 0: 0\n";
        cout << "количество ребер: 0\n";
        cout << "\nматрица смежности:\n";
        cout << "0\n";
        return;
    }
    
    const int MAX_ATTEMPTS = 1000;
    int attempt = 0;
    bool success = false;
    vector<int> finalDegrees;
    vector<int> actualDegrees;
    
    while (!success && attempt < MAX_ATTEMPTS) {
        attempt++;
        
        vector<int> degrees = generateDegrees(n, rng, directed);
        
        if (!directed) {
            if (!buildUndirected(degrees, graph.adjMatrix, rng)) {
                continue;
            }
        } else {
            if (!buildDirected(degrees, graph.adjMatrix, rng)) {
                continue;
            }
        }
        
        if (!isConnected(graph.adjMatrix)) {
            continue;
        }
        
        int edgeCount = countEdges(graph.adjMatrix, directed);
        
        success = true;
        finalDegrees = degrees;
        actualDegrees = computeActualDegrees(graph.adjMatrix, directed);
    }
    
    if (success) {
        cout << "\nграф успешно сгенерирован за " << attempt << " попыток\n";
        
        cout << "заданные степени вершин (по распределению):\n";
        for (int i = 0; i < n; i++) {
            cout << "вершина " << i << ": " << finalDegrees[i] << endl;
        }
        
        cout << "\nреальные степени вершин (по построенной матрице):\n";
        for (int i = 0; i < n; i++) {
            cout << "вершина " << i << ": " << actualDegrees[i] << endl;
        }
        
        int edgeCount = countEdges(graph.adjMatrix, directed);
        cout << "\nколичество ребер: " << edgeCount << endl;
        
        printMatrix(graph.adjMatrix, "матрица смежности");

    } else {
        cout << "\nне удалось сгенерировать граф за " << MAX_ATTEMPTS << " попыток\n";
        graph.reset();
    }
}

void calculateEccentricity(MyGraph& graph) {
    if (!graph.isGenerated) {
        cout << "сначала сгенерируйте граф\n";
        return;
    }
    
    int n = graph.verticesCount;
    
    if (n <= 1) {
        if (n == 0) {
            cout << "пустой граф\n";
            return;
        }
        cout << "эксцентриситет вершины 0: 0\n";
        cout << "\nрадиус графа: 0\n";
        cout << "диаметр графа: 0\n";
        cout << "центр графа: 0\n";
        cout << "диаметральные вершины: 0\n";
        return;
    }
    
    // инициализация матрицы расстояний
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                graph.distMatrix[i][j] = 0;
            } else if (graph.adjMatrix[i][j] == 1) {
                graph.distMatrix[i][j] = 1;
            } else {
                graph.distMatrix[i][j] = INF;
            }
        }
    }
    
    // алгоритм флойда для поиска кратчайших путей
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (graph.distMatrix[i][k] < INF && graph.distMatrix[k][j] < INF) {
                    if (graph.distMatrix[i][j] > graph.distMatrix[i][k] + graph.distMatrix[k][j]) {
                        graph.distMatrix[i][j] = graph.distMatrix[i][k] + graph.distMatrix[k][j];
                    }
                }
            }
        }
    }
    
    // вычисление эксцентриситетов
    vector<int> eccentricity(n, 0);
    int radius = INF;
    int diameter = 0;
    
    for (int i = 0; i < n; i++) {
        int maxDist = 0;
        for (int j = 0; j < n; j++) {
            if (graph.distMatrix[i][j] > maxDist && graph.distMatrix[i][j] < INF) {
                maxDist = graph.distMatrix[i][j];
            }
        }
        eccentricity[i] = maxDist;
        
        if (maxDist < radius) radius = maxDist;
        if (maxDist > diameter) diameter = maxDist;
        
        cout << "эксцентриситет вершины " << i << ": " << eccentricity[i] << endl;
    }
    
    cout << "\nрадиус графа: " << radius << endl;
    cout << "диаметр графа: " << diameter << endl;
    
    cout << "центр графа (вершины с эксцентриситетом = радиусу): ";
    for (int i = 0; i < n; i++) {
        if (eccentricity[i] == radius) {
            cout << i << " ";
        }
    }
    cout << endl;
    
    cout << "диаметральные вершины (эксцентриситет = диаметру): ";
    for (int i = 0; i < n; i++) {
        if (eccentricity[i] == diameter) {
            cout << i << " ";
        }
    }
    cout << endl;
}

// вспомогательная функция для поиска путей заданной длины
// тип поиска: 1 - мин, 2 - макс
vector<vector<int>> findPathsOfLength(const vector<vector<int>>& weightMatrix, int pathLength, int searchType) {
    int n = weightMatrix.size();
    vector<vector<int>> result(n, vector<int>(n, 0));
    
    if (pathLength == 1) {
        result = weightMatrix;
        return result;
    }
    
    vector<vector<int>> current = weightMatrix;
    int initValue = (searchType == 1) ? INF : -INF;
    
    for (int p = 1; p < pathLength; p++) {
        vector<vector<int>> next(n, vector<int>(n, 0));
        
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                bool found = false;
                int bestValue = initValue;
                
                for (int k = 0; k < n; k++) {
                    if (current[i][k] != 0 && weightMatrix[k][j] != 0) {
                        found = true;
                        int pathWeight = current[i][k] + weightMatrix[k][j];
                        
                        if (searchType == 1) {
                            if (pathWeight < bestValue) {
                                bestValue = pathWeight;
                            }
                        } else {
                            if (pathWeight > bestValue) {
                                bestValue = pathWeight;
                            }
                        }
                    }
                }
                
                if (found) {
                    next[i][j] = bestValue;
                }
            }
        }
        
        current = next;
    }
    
    result = current;
    return result;
}

void generateMatrix(MyGraph& graph, vector<vector<int>>& matrix, const string& matrixName, int weightType, RandomGenerator& rng) {
    int n = graph.verticesCount;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                matrix[i][j] = 0;
            } else if (graph.adjMatrix[i][j] == 1) {
                double x = PirsonDistribution(PARAM, rng);
                int value = (int)round(x) + 1;
                
                if (weightType == 1) {
                    value = abs(value);
                } else if (weightType == 2) {
                    value = -abs(value);
                } else if (weightType == 3) {
                    if (rng.getBool()) {
                        value = -value;
                    }
                }
                matrix[i][j] = value;
                if (!graph.isDirected) {
                    matrix[j][i] = value;
                }
            }
            else {
                matrix[i][j] = 0;
            }
        }
    }
    
    printMatrix(matrix, matrixName);
}

void generateWeightMatrix(MyGraph& graph, int weightType, RandomGenerator& rng) {
    generateMatrix(graph, graph.weightMatrix, "матрица весов", weightType, rng);
}

void generateCostMatrix(MyGraph& graph, int weightType, RandomGenerator& rng) {
    generateMatrix(graph, graph.costMatrix, "матрица стоимостей", 1, rng);
}

void initWeightMatrix(MyGraph& graph) {
    int weightType;
    cout << "выберите тип весов:\n";
    cout << "1 - только положительные\n";
    cout << "2 - только отрицательные\n";
    cout << "3 - смешанные\n";
    cout << "выбор: ";
    cin >> weightType;
    
    RandomGenerator rng;
    
    cout << "\nгенерация весовой матрицы:\n";
    generateWeightMatrix(graph, weightType, rng);
}

void shimbellMethod(MyGraph& graph) {
    if (!graph.isGenerated) {
        cout << "сначала сгенерируйте граф\n";
        return;
    }
    
    int n = graph.verticesCount;
    
    if (n <= 1) {
        if (n == 0) {
            cout << "пустой граф\n";
            return;
        }
        return;
    }
    
    int pathLength;
    
    cout << "введите длину пути (количество ребер): ";
    cin >> pathLength;

    if (pathLength == 0) {
        // единицы на главной диагонали, остальные 0
        vector<vector<int>> identityMatrix(n, vector<int>(n, 0));
        for (int i = 0; i < n; i++) {
            identityMatrix[i][i] = 1;
        }
        
        printMatrix(identityMatrix, "матрица минимальных путей");
        printMatrix(identityMatrix, "матрица максимальных путей");
        return;
    }
    
    if (pathLength < 0 || pathLength > n - 1) {
        cout << "длина пути должна быть положительной и не превышать n-1\n";
        return;
    }
    
    initWeightMatrix(graph);

    // поиск минимальных и максимальных путей
    vector<vector<int>> minResult = findPathsOfLength(graph.weightMatrix, pathLength, 1);
    vector<vector<int>> maxResult = findPathsOfLength(graph.weightMatrix, pathLength, 2);

    printMatrix(minResult, "матрица минимальных путей");
    printMatrix(maxResult, "матрица максимальных путей");
}

void countRoutes(MyGraph& graph) {
    if (!graph.isGenerated) {
        cout << "сначала сгенерируйте граф\n";
        return;
    }
    
    int n = graph.verticesCount;
    
    if (n == 0) {
        cout << "пустой граф\n";
        return;
    }
    
    int start, end;
    cout << "введите начальную вершину: ";
    cin >> start;
    cout << "введите конечную вершину: ";
    cin >> end;
    
    if (start < 0 || start >= n || end < 0 || end >= n) {
        cout << "неверный номер вершины\n";
        return;
    }
    
    if (n == 1) {
        if (start == 0 && end == 0) {
            cout << "\nмаршрут из вершины " << start << " в вершину " << end << " существует\n";
            cout << "общее количество маршрутов (включая путь длины 0): 1\n";
        } else {
            cout << "маршрут из вершины " << start << " в вершину " << end << " не существует\n";
        }
        return;
    }
    
    // сумма матриц смежности в степенях от 0 до n-1
    vector<vector<int>> sum(n, vector<int>(n, 0));
    
    for (int i = 0; i < n; i++) {
        sum[i][i] = 1;
    }
    
    // возводим в степени от 1 до n-1 и суммируем
    for (int p = 1; p < n; p++) {
        vector<vector<int>> power = matrixPower(graph.adjMatrix, p);
        
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                sum[i][j] += power[i][j];
            }
        }
    }
    
    // вывод матрицы сумм
    cout << "\nматрица сумм маршрутов (длины от 0 до " << n-1 << "):\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cout << sum[i][j] << " ";
        }
        cout << endl;
    }
    
    // вывод результата для конкретной пары вершин
    cout << "\nобщее количество маршрутов из вершины " << start 
         << " в вершину " << end << " (длины от 0 до " << n-1 << "): " 
         << sum[start][end] << endl;
    
    if (sum[start][end] == 0) {
        cout << "маршрутов не существует\n";
    }
}

void depthFirstSearch(const MyGraph& graph) {
    if (!graph.isGenerated) {
        cout << "сначала сгенерируйте граф\n";
        return;
    }

    int iterations = 0;
    int n = graph.verticesCount;
    
    if (n == 0) {
        cout << "пустой граф\n";
        return;
    }
    
    int startVertex;
    cout << "введите начальную вершину: ";
    cin >> startVertex;
    
    if (startVertex < 0 || startVertex >= n) {
        cout << "неверный номер вершины\n";
        return;
    }
    
    vector<int> visited(n, 0);
    
    stack<int> st;
    
    st.push(startVertex);
    
    visited[startVertex] = 1;
    
    cout << "\nобход графа в глубину (dfs):\n";
    cout << "последовательность вершин: ";
    
    while (!st.empty()) {
        int u = st.top();
        st.pop();
        
        cout << u << " ";
        
        for (int w = n - 1; w >= 0; w--) {
            if (graph.adjMatrix[u][w] == 1) { 
                if (visited[w] == 0) {
                    st.push(w);
                    visited[w] = 1;
                }
            }
            iterations++;
        }
    }
    cout << endl << "итераций совершено: " << iterations << endl;
}

// возвращает {dist[], prev[]} от источника
pair<vector<int>, vector<int>> dijkstra(int n, int source, const vector<vector<int>>& adjMatrix, const vector<vector<int>>& weightMatrix)
{
    vector<int> dist(n, INF);
    vector<int> prev(n, -1);
    vector<bool> inQueue(n, false);

    using pii = pair<int, int>;
    priority_queue<pii, vector<pii>, greater<pii>> pq;

    dist[source] = 0;
    pq.push({0, source});
    inQueue[source] = true;

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d != dist[u]) continue;
        inQueue[u] = false;

        for (int v = 0; v < n; v++) {
            if (adjMatrix[u][v] == 1) {
                int w = weightMatrix[u][v];
                if (dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    prev[v] = u;
                    if (!inQueue[v]) {
                        pq.push({dist[v], v});
                        inQueue[v] = true;
                    }
                }
            }
        }
    }

    return {dist, prev};
}

void dijkstraUI(MyGraph& graph) {
    if (!graph.isGenerated) {
        cout << "сначала сгенерируйте граф\n";
        return;
    }

    int n = graph.verticesCount;
    if (n == 0) {
        cout << "пустой граф\n";
        return;
    }

    initWeightMatrix(graph);

    int start, end;
    cout << "введите начальную вершину: ";
    cin >> start;
    cout << "введите конечную вершину: ";
    cin >> end;

    if (start < 0 || start >= n || end < 0 || end >= n) {
        cout << "неверный номер вершины\n";
        return;
    }

    auto [dist, prev] = dijkstra(n, start, graph.adjMatrix, graph.weightMatrix);

    cout << "\nвектор расстояний от вершины " << start << ":\n";
    for (int i = 0; i < n; i++) {
        cout << "вершина " << i << ": ";
        if (dist[i] == INF) cout << "INF";
        else cout << dist[i];
        cout << endl;
    }

    if (dist[end] == INF) {
        cout << "\nпуть из вершины " << start << " в вершину " << end << " не существует\n";
        return;
    }

    vector<int> path;
    for (int v = end; v != -1; v = prev[v])
        path.push_back(v);
    reverse(path.begin(), path.end());

    cout << "\nкратчайший путь из вершины " << start << " в вершину " << end << ":\n";
    cout << "расстояние: " << dist[end] << endl;
    cout << "путь: ";
    for (size_t i = 0; i < path.size(); i++) {
        cout << path[i];
        if (i < path.size() - 1) cout << " -> ";
    }
    cout << endl;
}

struct LabelInfo {
    char sign;
    int neighbor;
    int delta;
};

void fordFulkerson(MyGraph& graph) {
    if (!graph.isGenerated) {
        cout << "сначала сгенерируйте граф\n";
        return;
    }

    int n = graph.verticesCount;
    if (n == 0) return;

    RandomGenerator rng;
    generateMatrix(graph, graph.capacityMatrix, "матрица пропускных способностей", 1, rng);

    int s, t;
    cout << "введите источник: ";
    cin >> s;
    cout << "введите сток: ";
    cin >> t;

    if (s < 0 || s >= n || t < 0 || t >= n || s == t) {
        cout << "неверные вершины\n";
        return;
    }

    vector<vector<int>> F(n, vector<int>(n, 0));
    vector<LabelInfo> P(n);
    vector<int> S(n);
    vector<int> N(n);
    const int INF_VAL = 1e9;

M:
    for (int v = 0; v < n; v++) {
        S[v] = 0;
        N[v] = 0;
        P[v] = {'+', -1, 0};
    }
    S[s] = 1;
    P[s] = {'+', -1, INF_VAL};

    int a = 0;
    
    while (true) {
        a = 0;
        
        for (int v = 0; v < n; v++) {
            if (S[v] == 1 && N[v] == 0) {
                for (int u = 0; u < n; u++) {
                    if (graph.adjMatrix[v][u] == 1) {
                        if (S[u] == 0 && F[v][u] < graph.capacityMatrix[v][u]) {
                            S[u] = 1;
                            int delta = min(P[v].delta, graph.capacityMatrix[v][u] - F[v][u]);
                            P[u] = {'+', v, delta};
                            a = 1;
                        }
                    }
                }
                
                for (int u = 0; u < n; u++) {
                    if (graph.adjMatrix[u][v] == 1) {
                        if (S[u] == 0 && F[u][v] > 0) {
                            S[u] = 1;
                            int delta = min(P[v].delta, F[u][v]);
                            P[u] = {'-', v, delta};
                            a = 1;
                        }
                    }
                }
                
                N[v] = 1;
            }
        }
        
        if (S[t] == 1) {
            int delta = P[t].delta;
            int x = t;
            while (x != s) {
                int p_node = P[x].neighbor;
                if (P[x].sign == '+') {
                    F[p_node][x] += delta;
                } else {
                    F[x][p_node] -= delta;
                }
                x = p_node;
            }
            goto M;
        }
        
        if (a == 0) break;
    }

    int maxFlow = 0;
    for (int j = 0; j < n; j++) {
        maxFlow += F[s][j];
    }

    graph.flowMatrix = F;
    graph.lastMaxFlow = maxFlow;
    graph.lastSource = s;
    graph.lastSink = t;

    cout << "\nмаксимальный поток: " << maxFlow << endl;
    printMatrix(F, "матрица потока F");
}

pair<vector<int>, vector<int>> bellmanMoore(
    int n, int source,
    const vector<vector<int>>& adjMatrix,
    const vector<vector<int>>& costMatrix)
{
    vector<int> dist(n, INF);
    vector<int> prev(n, -1);
    vector<bool> inQueue(n, false);

    dist[source] = 0;
    queue<int> q;
    q.push(source);
    inQueue[source] = true;

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        inQueue[u] = false;

        for (int v = 0; v < n; v++) {
            if (adjMatrix[u][v] == 1 && dist[u] < INF) {
                int nd = dist[u] + costMatrix[u][v];
                if (nd < dist[v]) {
                    dist[v] = nd;
                    prev[v] = u;
                    if (!inQueue[v]) {
                        q.push(v);
                        inQueue[v] = true;
                    }
                }
            }
        }
    }

    return {dist, prev};
}

void minCostFlowUI(MyGraph& graph, RandomGenerator& rng) {
    if (!graph.isGenerated) {
        cout << "сначала сгенерируйте граф\n";
        return;
    }
    if (graph.lastMaxFlow <= 0) {
        cout << "сначала выполните алгоритм Форда-Фалкерсона (пункт 7)\n";
        return;
    }

    int n = graph.verticesCount;
    int s = graph.lastSource;
    int t = graph.lastSink;
    int target = (2 * graph.lastMaxFlow) / 3;

    cout << "\nмаксимальный поток (из предыдущего запуска): " << graph.lastMaxFlow << endl;
    cout << "целевой поток [2/3 * max] = " << target << endl;

    if (target == 0) {
        cout << "целевой поток равен 0, вычисление не требуется\n";
        return;
    }

    generateCostMatrix(graph, 1, rng);

    vector<vector<int>> flow(n, vector<int>(n, 0));
    int currentFlow = 0;
    int totalCost = 0;

    while (currentFlow < target) {
        vector<vector<int>> modAdj(n, vector<int>(n, 0));
        vector<vector<int>> modCap(n, vector<int>(n, 0));
        vector<vector<int>> modCost(n, vector<int>(n, 0));
        bool hasNegative = false;

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (graph.adjMatrix[i][j] == 1) {
                    if (flow[i][j] > 0) {
                        modAdj[j][i] = 1;
                        modCap[j][i] = flow[i][j];
                        modCost[j][i] = -graph.costMatrix[i][j];
                        hasNegative = true;
                    }
                    if (flow[i][j] < graph.capacityMatrix[i][j]) {
                        modAdj[i][j] = 1;
                        modCap[i][j] = graph.capacityMatrix[i][j] - flow[i][j];
                        modCost[i][j] = graph.costMatrix[i][j];
                    }
                }
            }
        }

        auto [dist, prev] = hasNegative ? bellmanMoore(n, s, modAdj, modCost) : dijkstra(n, s, modAdj, modCost);

        if (dist[t] == INF) break;

        int delta = target - currentFlow;
        for (int x = t; x != s; x = prev[x])
            delta = min(delta, modCap[prev[x]][x]);

        for (int x = t; x != s; x = prev[x]) {
            int p = prev[x];
            if (modCost[p][x] >= 0) {
                flow[p][x] += delta;
                totalCost += delta * graph.costMatrix[p][x];
            } else {
                flow[x][p] -= delta;
                totalCost -= delta * graph.costMatrix[x][p];
            }
        }
        currentFlow += delta;
    }

    if (currentFlow < target)
        cout << "\nне удалось достичь целевого потока. достигнуто: " << currentFlow << endl;
    else
        cout << "\nпоток минимальной стоимости найден\n";

    cout << "величина потока: " << currentFlow << endl;
    cout << "суммарная стоимость: " << totalCost << endl;
    printMatrix(flow, "матрица потока минимальной стоимости");
}

double determinant(vector<vector<double>> mat) {
    int n = mat.size();
    if (n == 0) return 1.0;
    double det = 1.0;

    for (int col = 0; col < n; col++) {
        int pivot = -1;
        for (int row = col; row < n; row++) {
            if (fabs(mat[row][col]) > 1e-9) { pivot = row; break; }
        }
        if (pivot == -1) return 0.0;

        if (pivot != col) {
            swap(mat[pivot], mat[col]);
            det *= -1;
        }

        det *= mat[col][col];
        for (int row = col + 1; row < n; row++) {
            double f = mat[row][col] / mat[col][col];
            for (int k = col; k < n; k++)
                mat[row][k] -= f * mat[col][k];
        }
    }
    return det;
}

void kirchhoff(MyGraph& graph) {
    if (!graph.isGenerated) {
        cout << "сначала сгенерируйте граф\n";
        return;
    }
    if (graph.isDirected) {
        cout << "теорема Кирхгофа применяется только к неориентированным графам\n";
        return;
    }

    int n = graph.verticesCount;

    if (n <= 1) {
        cout << "\nчисло остовных деревьев: 1\n";
        return;
    }

    vector<vector<int>> B(n, vector<int>(n, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (i != j && graph.adjMatrix[i][j] == 1) {
                B[i][j] = -1;
                B[i][i]++;
            }

    printMatrix(B, "матрица Кирхгофа B(G)");

    vector<vector<double>> minor(n - 1, vector<double>(n - 1));
    for (int i = 1; i < n; i++)
        for (int j = 1; j < n; j++)
            minor[i - 1][j - 1] = B[i][j];

    long long result = (long long)round(determinant(minor));
    cout << "\nчисло остовных деревьев: " << result << endl;
}

int main() {
    RandomGenerator rng;
    MyGraph graph;
    
    while (true) {
        cout << "\nменю:\n";
        cout << "1. сгенерировать граф\n";
        cout << "2. найти центр и диаметр\n";
        cout << "3. метод шимбелла \n";
        cout << "4. подсчет маршрутов\n";
        cout << "5. обход графа в глубину (dfs)\n";
        cout << "6. поиск кратчайшего пути (Дейкстра)\n";
        cout << "7. алгоритм Форда-Фалкерсона\n";
        cout << "8. поток минимальной стоимости [2/3 * max]\n";
        cout << "9. найти число остовных деревьев по т. Кирхгофа\n";
        cout << "0. выход\n";
        cout << "выбор: ";
        
        int choice;
        cin >> choice;
        
        switch(choice) {
            case 1:
                generateGraph(graph, rng);
                break;
            case 2:
                if (graph.isGenerated) calculateEccentricity(graph);
                else cout << "сначала сгенерируйте граф\n";
                break;
            case 3:
                if (graph.isGenerated) shimbellMethod(graph);
                else cout << "сначала сгенерируйте граф\n";
                break;
            case 4:
                if (graph.isGenerated) countRoutes(graph);
                else cout << "сначала сгенерируйте граф\n";
                break;
            case 5:
                if (graph.isGenerated) depthFirstSearch(graph);
                else cout << "сначала сгенерируйте граф\n";
                break;
            case 6:
                if (graph.isGenerated) dijkstraUI(graph);
                else cout << "сначала сгенерируйте граф\n";
                break;
            case 7:
                if (graph.isGenerated) fordFulkerson(graph);
                else cout << "сначала сгенерируйте граф\n";
                break;
            case 8:
                minCostFlowUI(graph, rng);
                break;
            case 9:
                kirchhoff(graph);
                break;
            case 0:
                return 0;
            default:
                cout << "неверный выбор\n";
        }
    }
    
    return 0;
}