#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <queue>
#include <stack>

using namespace std;

const int INF = 10000000;
const double PARAM = 3.0;

struct MyGraph {
    vector<vector<int>> adjMatrix;
    vector<vector<int>> distMatrix;
    vector<vector<int>> weightMatrix;
    int verticesCount;
    bool isGenerated;
    bool isDirected;
    
    MyGraph() : verticesCount(0), isGenerated(false), isDirected(false) {}
    
    void initMatrices(int n, bool directed = false) {
        verticesCount = n;
        isDirected = directed;
        adjMatrix.assign(n, vector<int>(n, 0));
        distMatrix.assign(n, vector<int>(n, INF));
        weightMatrix.assign(n, vector<int>(n, 0));
        isGenerated = true;
    }
    
    void reset() {
        verticesCount = 0;
        isGenerated = false;
        isDirected = false;
        adjMatrix.clear();
        distMatrix.clear();
        weightMatrix.clear();
    }
};

class RandomGenerator {
private:
    mt19937 gen;
    normal_distribution<> normalDistrubution;

public:
    RandomGenerator() : gen(random_device{}()), normalDistrubution(0.0, 1) {}
    
    double getNormal() {
        return normalDistrubution(gen);
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

// проверка связности через возведение матрицы в степени
bool isConnected(const vector<vector<int>>& adjMatrix) {
    int n = adjMatrix.size();
    if (n <= 1) return true;
    
    vector<vector<int>> sum(n, vector<int>(n, 0));
    
    for (int p = 1; p < n; p++) {
        vector<vector<int>> power = matrixPower(adjMatrix, p);
        
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (power[i][j] > 0) {
                    sum[i][j] = 1;
                }
            }
        }
    }
    
    for (int j = 1; j < n; j++) {
        if (sum[0][j] == 0) return false;
    }
    
    return true;
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

bool buildDirected(const vector<int>& outDegrees, vector<vector<int>>& adjMatrix) {
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
        
        for (int j = i + 1; j < n && need > 0; j++) {
            adjMatrix[i][j] = 1;
            need--;
        }
    }
    
    return true;
}

bool buildUndirected(const vector<int>& degrees, vector<vector<int>>& adjMatrix) {
    int n = degrees.size();
    if (n == 0) return true;
    if (n == 1) {
        if (degrees[0] == 0) {
            adjMatrix[0][0] = 0;
            return true;
        }
        return false;
    }
    
    buildDirected(degrees, adjMatrix);

    
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
            if (!buildUndirected(degrees, graph.adjMatrix)) {
                continue;
            }
        } else {
            if (!buildDirected(degrees, graph.adjMatrix)) {
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
        
        cout << "\nматрица смежности:\n";
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                cout << graph.adjMatrix[i][j] << " ";
            }
            cout << endl;
        }
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

// вспомогательная функция для вывода матрицы
void printMatrix(const vector<vector<int>>& matrix, const string& name, int pathLength) {
    int n = matrix.size();
    cout << "\n" << name << " длины " << pathLength << ":\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (matrix[i][j] == 0 && i != j) {
                cout << "0 ";
            } else {
                cout << matrix[i][j] << " ";
            }
        }
        cout << endl;
    }
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
        cout << "граф с одной вершиной: все пути нулевые\n";
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
        
        printMatrix(identityMatrix, "матрица минимальных путей", pathLength);
        printMatrix(identityMatrix, "матрица максимальных путей", pathLength);
        return;
    }
    
    
    if (pathLength < 0) {
        cout << "длина пути должна0быть положительной\n";
        return;
    }
    
    RandomGenerator rng;
    
    cout << "\nгенерация весовой матрицы:\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                graph.weightMatrix[i][j] = 0;
            } else if (graph.adjMatrix[i][j] == 1) {
                double x = PirsonDistribution(PARAM, rng);
                int weight = (int)round(x) + 1;
                graph.weightMatrix[i][j] = weight;
                if (!graph.isDirected) {
                    graph.weightMatrix[j][i] = weight;
                }
            } else {
                graph.weightMatrix[i][j] = 0;
            }
        }
    }
    
    // вывод весовой матрицы
    cout << "\nвесовая матрица Ω:\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cout << graph.weightMatrix[i][j] << " ";
        }
        cout << endl;
    }
    
    // поиск минимальных и максимальных путей
    vector<vector<int>> minResult = findPathsOfLength(graph.weightMatrix, pathLength, 1);
    vector<vector<int>> maxResult = findPathsOfLength(graph.weightMatrix, pathLength, 2);

    printMatrix(minResult, "матрица минимальных путей", pathLength);
    printMatrix(maxResult, "матрица максимальных путей", pathLength);
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
    
    int start, end, length;
    cout << "введите начальную вершину: ";
    cin >> start;
    cout << "введите конечную вершину: ";
    cin >> end;
    cout << "введите длину маршрута: ";
    cin >> length;
    
    if (start < 0 || start >= graph.verticesCount || end < 0 || end >= graph.verticesCount) {
        cout << "неверный номер вершины\n";
        return;
    }
    
    if (length < 1) {
        cout << "длина маршрута должна быть положительной\n";
        return;
    }
    
    if (n == 1) {
        if (start == 0 && end == 0) {
            cout << "\nколичество маршрутов длины " << length 
                 << " из вершины " << start << " в вершину " << end << ": " 
                 << (length == 0 ? 1 : 0) << endl;
        }
        return;
    }
    
    if (length > graph.verticesCount - 1) {
        cout << "длина маршрута введена немерно\n";
        return;
    }
    
    // возведение матрицы смежности в степень length
    vector<vector<int>> power = matrixPower(graph.adjMatrix, length);
    
    // вывод всей матрицы в степени length
    cout << "\nматрица смежности в степени " << length << ":\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cout << power[i][j] << " ";
        }
        cout << endl;
    }
    
    // вывод значения для конкретной пары вершин
    cout << "\nколичество маршрутов длины " << length 
         << " из вершины " << start << " в вершину " << end << ": " 
         << power[start][end] << endl;
    
    if (power[start][end] == 0) {
        cout << "маршрутов длины " << length << " не существует\n";
    }
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
            case 0:
                return 0;
            default:
                cout << "неверный выбор\n";
        }
    }
    
    return 0;
}