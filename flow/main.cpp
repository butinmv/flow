#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

using namespace std;
using namespace sf;

void reverse(char str[], int length)
{
    int start = 0;
    int end = length -1;
    while (start < end)
    {
        swap(*(str+start), *(str+end));
        start++;
        end--;
    }
}
char* itoa(int num, char* str, int base)
{
    int i = 0;
    bool isNegative = false;
    
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    
    if (num < 0 && base == 10)
    {
        isNegative = true;
        num = -num;
    }
    
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }
    
    if (isNegative)
        str[i++] = '-';
    str[i] = '\0';
    reverse(str, i);
    return str;
}


// ПП
void inpFile();
void work(); // функция поиска максимального потока
int work(int wgnode); // обработка одной вершины

// ребро
struct Edge
{
    int start; // первая вершина
    int end; // вторая вершина
    int stream; // вместимость потока
    int used_stream; // использованый поток
    
    // конструктор ребра
    Edge(int a, int b, int c)
    {
        start = a;
        end = b;
        stream = c;
        used_stream = 0;
    }
};

//цвет интерфейса
Color white = Color(255, 255, 255);
Color white_100 = Color(255, 255, 255, 100);
Color white_255 = Color(255, 255, 255, 255);
Color blue_background = Color(35, 95, 165);
Color line_color = Color(255, 255, 255, 50);
Color noCycle = Color(24, 67, 117);

vector<int> g; // вершины и связи
vector<int> x, y; // координаты вершин
vector<Edge *> bufferVertex; // список ребер

vector<int> e; // переполнения вершин
vector<int> h; // высота вершины
vector <vector <Edge *> *> wg; // работаем с этим списком вершин
int start_stream, end_stream; // стартовая и конечная вершина

int r = 700;                            // радиус круга



int main()
{
    // задаем круг для вершины
    int r = 30;
    
    // белый круг, используется в цикле
    CircleShape shape(r);
    shape.setOutlineColor(white_255);
    shape.setOutlineThickness(5);
    shape.setFillColor(Color(255, 255, 255, 0));

    
    
    // задаем прямоугольник и треугольник для ребра
    RectangleShape line;
    line.setFillColor(white_255);
    ConvexShape triangle;
    triangle.setPointCount(3);
    triangle.setFillColor(white_255);
    
    
    // найтсроки шрифта
    Font font;
    if (!font.loadFromFile("cour.ttf")) {
        return EXIT_FAILURE;
    }
    Text text("", font, 35);
    text.setFillColor(white_255);
    text.setStyle(Text::Bold);
    
    
    // окно
    ContextSettings settings;
    settings.antialiasingLevel = 15;
    string name = "Flow";
    VideoMode videoMode;
    videoMode.width = 2000;
    videoMode.height = 2000;
    RenderWindow *window = new RenderWindow(videoMode, name, Style::Close, settings);
    
    // настройки иконки
    Image icon;
    if (!icon.loadFromFile("icon.png")) {
        return EXIT_FAILURE;
    }
    window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    
    
    // функция программы
    work();
    
    //cout << "while" << endl;
    
    // главный цикл программы
    while (window->isOpen())
    {
        // обработка событий окна
        Event event;
        while (window->pollEvent(event))
        {
            // окно закрыли
            if (event.type == Event::Closed)
                window->close();
        }
        
        // двигаем узлы мышкой
        if (Mouse::isButtonPressed(Mouse::Left))
        {
            int n = g.size();
            for (int i = 0; i < n; i++)
                if (x[i] <= (Mouse::getPosition(*window)).x && x[i] >= (Mouse::getPosition(*window)).x - 2 * r &&
                    y[i] <= (Mouse::getPosition(*window)).y && y[i] >= (Mouse::getPosition(*window)).y - 2 * r)
                {
                    x[i] = (Mouse::getPosition(*window)).x - r;
                    y[i] = (Mouse::getPosition(*window)).y - r;
                    break;
                }
        }
        
        // очистка окна
        window->clear(blue_background);
        
        // вывод графа
        // ребер
        int size = bufferVertex.size();
        for (int i = 0; i < size; i++)
        {
            int a = bufferVertex[i]->start;
            int b = bufferVertex[i]->end;
            
            float h_ab, w_ab, long_ab, angle;
            h_ab = x[a] - x[b];
            w_ab = y[a] - y[b];
            // гипотенуза
            long_ab = sqrt(h_ab * h_ab + w_ab * w_ab);
            
            // угол
            angle = asin(h_ab / long_ab) / M_PI * 180;
            if(h_ab < 0 && w_ab > 0)
                angle = 180 - angle;
            if(h_ab >= 0 && w_ab >= 0)
                angle = 180 - angle;
            
            // задаем угол
            line.setRotation(angle);
            // размер линии
            line.setSize(Vector2f(4, long_ab));
            // от куда идет линия
            line.setPosition(Vector2f(x[a] + r, y[a] + r));
            // треугольник
            // определяем вершины
            int r = 30, rr = 60;
            triangle.setPoint(0, sf::Vector2f(x[b] + r, y[b] + r));
            angle = angle - 90;
            triangle.setPoint(1, sf::Vector2f(x[b] + rr * cos((angle + 20) * M_PI / 180) + r, y[b] + rr * sin((angle + 20) * M_PI / 180) + r));
            triangle.setPoint(2, sf::Vector2f(x[b] + rr * cos((angle - 20) * M_PI / 180) + r, y[b] + rr * sin((angle - 20) * M_PI / 180) + r));
            // вывод пропускной способности
            char s[5] = "";
            itoa(bufferVertex[i]->used_stream, s, 10);
            string str = s, str2;
            str = "[" + str + "/";
            itoa(bufferVertex[i]->stream, s, 10);
            str2 = s;
            str = str + str2 + "]";
            text.setString(str);
            rr = rr + 30;
            text.setPosition(x[b] + rr * cos((angle + 15) * M_PI / 180), y[b] + rr * sin((angle + 15) * M_PI / 180));
            
            window->draw(line);
            window->draw(triangle);
            window->draw(text);
        }
        
        
        // вершин
        int n = g.size();
        for (int i = 0; i < n; i++)
        {
            shape.setPosition(x[i], y[i]);
            char s[5] = "";
            itoa(i + 1, s, 10);
            string str = s;
            
            if(i == start_stream)
            {
                str = str + " :begin";
                shape.setOutlineColor(Color::Red);
            }
            if(i == end_stream)
            {
                str = str + " :end";
                shape.setOutlineColor(Color::Red);
            }
            text.setString(str);
            if(i + 1 < 10)
                text.setPosition(x[i] + 70, y[i] + 6);
            else
                text.setPosition(x[i] + 30, y[i] + 6);
            
            window->draw(shape);
            window->draw(text);
            shape.setFillColor(white_100);
            
            
            int v = 40;
            int size = videoMode.width / v + 1;
            for(int j = 0; j < size; j++)
            {
                Vertex line[] =
                {
                    Vertex(Vector2f(j * v, 0), line_color),
                    Vertex(Vector2f(j * v, videoMode.height), line_color)
                };
                window->draw(line, 2, Lines);
            }
            
            size = videoMode.height / v + 1;
            for(int j = 0; j < size; j++)
            {
                Vertex line[] =
                {
                    Vertex(Vector2f(0, j * v), line_color),
                    Vertex(Vector2f(videoMode.width, j * v), line_color)
                };
                window->draw(line, 2, Lines);
            }
        }
        window->display();
    }
    return 0;
}

// ввод
void inpFile()
{
    x.clear(); // очистка координат x
    y.clear(); // очистка координат y
    
    int size = g.size(); // очистка вершин
    g.clear();
    
    size = bufferVertex.size(); // очистка ребер
    for (int i = 0; i < size; i++)
        delete bufferVertex[i];
    bufferVertex.clear();
    
    e.clear();
    h.clear();
    
    size = wg.size(); // очистка вершин
    for (int i = 0; i < size; i++)
        wg[i]->clear();
    wg.clear();
    
    // ввод вершин
    FILE *file;
    int n;
    file = fopen("input.txt", "r");
    
    fscanf(file, "%d", &start_stream);
    fscanf(file, "%d", &end_stream);
    start_stream--;
    end_stream--;
    
    fscanf(file, "%d", &n);
    for (int i = 0; i < n; i++)
    {
        g.push_back(i);
        e.push_back(0);
        h.push_back(0);
        wg.push_back(new vector<Edge *>());
    }
    h[start_stream] = n;
    
    
    // вывод по кругу
    for (int i = 0; i < n; i++) {
        int xx = 1000 - 30 + r * cos(i * 360 / n * M_PI / 180);
        int yy = 900 - 30 + r * sin(i * 360 / n * M_PI / 180);
        x.push_back(xx);
        y.push_back(yy);
    }
    
    // ввод ребер
    int p1, p2, p3;
    while(!feof(file))
        if(fscanf(file, "%d", &p1))
            if(fscanf(file, "%d", &p2))
                if(fscanf(file, "%d", &p3))
                    if (p1 > 0 && p1 <= n && p2 > 0 && p2 <= n && p3 > 0)
                    {
                        Edge* a = new Edge(p1 - 1, p2 - 1, p3);
                        wg[p1 - 1]->push_back(a);
                        wg[p2 - 1]->push_back(a);
                        bufferVertex.push_back(a);
                    }
    
    // вывод
    printf("\n----- Graph and Fin -----\n");
    size = bufferVertex.size();
    for(int i = 0; i < size; i++)
        printf("%d-%d ", bufferVertex[i]->start + 1, bufferVertex[i]->end + 1);
    printf("\n");
}

// вывод в консоль
void display()
{
    int size = e.size();
    printf("\n e| weight\n");
    for(int i = 0; i < size; i++)
        printf("%2d|%2d\n", i + 1, e[i]);
    
    printf("\n");
    
    size = h.size();
    printf("\n h| height\n");
    for(int i = 0; i < size; i++)
        printf("%2d|%2d\n", i + 1, h[i]);
    
    size = wg.size();
    printf("\nwg| fin\n");
    for(int i = 0; i < size; i++)
    {
        printf("%2d|", i + 1);
        int wgsize = wg[i]->size();
        for(int j = 0; j < wgsize; j++)
            printf(" %d-%d(%d/%d)", (*wg[i])[j]->start + 1, (*wg[i])[j]->end + 1, (*wg[i])[j]->used_stream, (*wg[i])[j]->stream);
        printf("\n");
    }
    
    printf("\n");
    
    size = g.size();
    printf("\ng| ");
    for(int i = 0; i < size; i++)
        printf("%d ", g[i] + 1);
    printf("\n-------------------------\n");
}

// функция программы
void work()
{
    // ввод из файла
    inpFile();
    
    printf("start: %d, end: %d\n", start_stream + 1, end_stream + 1);
    
    // алгоритм
    // входные
    // заполняем ребра вышедшие из стартовой
    int size = wg[start_stream]->size();
    for(int i = 0; i < size; i++)
    {
        Edge* a = (*wg[start_stream])[i];
        if(a->start == start_stream)
        {
            e[a->end] = e[a->end] + a->stream;
            a->used_stream = a->stream;
        }
    }
    
    display();
    
    // один обход
    bool found = true;
    int gsize = g.size();
    
    // работаем по списку вершин пока можем "выполнять подъем и в начало"
    while(found)
    {
        found = false;
        for(int i = gsize - 1; i > -1; i--)
        {
            int a = g[i];
            
            printf("work with %d -----", a + 1);
            int x = work(a);
            printf(" hh = %d\n", x);
            
            // подъем
            if(x > 0)
            {
                if(x == 2)
                {
                    g.erase(g.begin() + i);
                    g.push_back(a);
                    
                    //wg.erase(wg.begin() + i);
                    //wg.push_back(node);
                }
                
                found = true;
                break;
            }
        }
        
        display();
    }
}

// проталкивание и сброс в одной вершине
int work(int wgnode)
{
    int hh = 0;
    int size = wg[wgnode]->size();
    if(wgnode != start_stream || e[wgnode] != 0)
    {
        for(int i = 0; i < size; i++)
        {
            Edge* a = (*wg[wgnode])[i];
            // выходящее ребро
            // проталкивание
            if(e[wgnode] > 0)
                if(a->start == wgnode && a->end != start_stream)
                {
                    int raz = a->stream - a->used_stream;
                    if(raz < e[a->start])
                    {
                        a->used_stream = a->stream; // ребро заполнено
                        e[a->end] = e[a->end] + raz; // пришедший поток
                        e[a->start] = e[a->start] - raz; // ушедший поток
                        if(h[a->start] <= h[a->end])
                        {
                            h[a->start] = h[a->end] + 1;
                            hh = 2;
                        }
                    }
                    else
                        if(a->end != start_stream)
                        {
                            a->used_stream = a->used_stream + e[a->start]; // ребро заполнено
                            e[a->end] = e[a->end] + e[a->start]; // пришедший поток
                            e[a->start] = 0; // ушедший поток
                            if(h[a->start] <= h[a->end])
                            {
                                h[a->start] = h[a->end] + 1;
                                hh = 2;
                            }
                        }
                }
        }
        
        // слив ненужного
        int eee = e[wgnode];
        if(e[wgnode] > 0 && wgnode != end_stream)
        {
            printf("eee = %d\n", eee);
            
            size = wg[wgnode]->size();
            for(int i = 0; i < size; i++)
            {
                Edge* a = (*wg[wgnode])[i];
                
                int raz = a->used_stream;
                
                // входящее ребро
                if(a->end == wgnode)
                {
                    printf("input: %d-%d\n", a->start, a->end);
                    if(raz < e[wgnode])
                    {
                        a->used_stream = 0; // ребро пустое
                        e[wgnode] = e[wgnode] - raz; // пришедший поток
                        if(a->start != start_stream)
                            e[a->start] = e[a->start] + raz; // ушедший поток
                        if(hh == 0)
                            hh = 1;
                    }
                    else
                    {
                        a->used_stream = a->used_stream - e[wgnode]; // ребро заполнено
                        if(a->start != start_stream)
                            e[a->start] = e[a->start] + e[wgnode]; // пришедший поток
                        e[wgnode] = 0; // ушедший поток
                        if(hh == 0)
                            hh = 1;
                    }
                }
            }
        }
    }
    return hh;
}
