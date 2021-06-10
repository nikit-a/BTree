#include <iostream>
#include <utility>
#include <vector>
#include <fstream>
#include <string>

using namespace std;
// параметр t вводится с консоли ограничивает количество ключей
static int t;
// счетчик для именования файлов
static int counter = 1;
// путь до папки со всеми узлами задается в консоли
static string defaultPath;
// здесь лежит итоговый результат для вывода в файл
static string result;

class Node {
private:
    // количество ключей
    int countKeys{};
    // сами ключи
    vector<int> keys;
    // значения ключей
    vector<int> values;
    // является ли узел листиком
    bool isList{};
    // ссылки на дочерние узлы
    vector<string> childrenNodes;
    // путь до файла где хранятся данные о данном узле
    string filePath;
    // путь к родителю
    string fileParent;

public:
    // создание новой ноды с уникальным именем файла
    Node(string fileParent, int flag) {
        // проверка что нужен конструктор создания ноды а не копирования
        if (flag == 0) {
            this->fileParent = std::move(fileParent);
            isList = true;
            childrenNodes.emplace_back("nil");
            filePath = defaultPath + to_string(counter) + ".bin";
            counter++;
        }

    }

    // для уже созданных узлов (хранящихся в файле), чтобы их инициализировать из файла
    explicit Node(string fileName) {
        filePath = std::move(fileName);
    }

    Node()
    = default;

    string &getFileParent() {
        return fileParent;
    }

    string getFilePath() {
        return filePath;
    }

    int getCountKeys() const {
        return countKeys;
    }

    void setCountKeys(int value) {
        countKeys = value;
    }

    vector<int> &getKeys() {
        return keys;
    }

    vector<int> &getValues() {
        return values;
    }

    bool getIsList() const {
        return isList;
    }

    void setIsList(bool value) {
        isList = value;
    }

    vector<string> &getChildren() {
        return childrenNodes;
    }

    /*
     * Очищаем информацию об узле перед чтением из файла
     */
    void clearInfo() {
        values.clear();
        keys.clear();
        childrenNodes.clear();
        countKeys = 0;
        fileParent.clear();
    }

    /*
     * Читаем информацию об узле из бинарного файла и инициализируем поля
     */
    void readFile(const string &path) {
        clearInfo();
        //Открываем файл в двоичном режиме только для чтения
        ifstream in(path, ios::binary | ios::in);
        // считали количество ключей
        in.read((char *) &countKeys, sizeof countKeys);
        // Читаем элементы вектора и записываем в вектор
        for (int i = 0; i < countKeys; ++i) {
            int key;
            in.read((char *) (&key), sizeof(key));
            keys.push_back(key);
        }
        // Читаем элементы вектора и записываем в вектор
        for (int i = 0; i < countKeys; ++i) {
            int value;
            in.read((char *) (&value), sizeof(value));
            values.push_back(value);
        }
        in.read((char *) &isList, sizeof isList);
        int size = 0;
        // Читаем и записываем размер вектора
        in.read((char *) (&size), sizeof(size));
        // Читаем элементы вектора и записываем в вектор
        for (int i = 0; i < size; ++i) {
            string children;
            int sizeItem;
            in.read((char *) (&sizeItem), sizeof(sizeItem));
            char symbol;
            for (int j = 0; j < sizeItem; ++j) {
                in.read((char *) (&symbol), sizeof(symbol));
                children += symbol;
            }
            childrenNodes.push_back(children);
        }
        in.read((char *) &filePath, sizeof filePath.size());
        int sizeParent = 0;
        in.read((char *) &sizeParent, sizeof sizeParent);
        char symbol;
        for (int j = 0; j < sizeParent; ++j) {
            in.read((char *) (&symbol), sizeof(symbol));
            fileParent += symbol;
        }
        // закрываем файл
        in.close();
    }

    /*
     * запись информации об узле в бинарном виде
     */
    void writeToFile(const string &path) {
        //Открываем файл в двоичном режиме для записи
        ofstream out(path, ios::binary | ios::out);
        // Записываем количество ключей в узле
        out.write((char *) &countKeys, sizeof countKeys);
        // записываем сами ключи
        for (int i = 0; i < countKeys; ++i) {
            out.write((char *) (&keys[i]), sizeof(int));
        }
        // записываем сами значения
        for (int i = 0; i < countKeys; ++i) {
            out.write((char *) (&values[i]), sizeof(int));
        }
        // записываем является ли узел листиком
        out.write((char *) &isList, sizeof isList);
        // записываем ссылки на дочерние узлы(пути до файлов)
        int sizeChildren = childrenNodes.size();
        out.write((char *) &sizeChildren, sizeof sizeChildren);
        for (auto &item : childrenNodes) {
            int sizeItem = item.size();
            out.write((char *) &sizeItem, sizeof sizeItem);
            for (char &i : item) {
                out.write((char *) (&i), sizeof(char));
            }
        }
        // запись пути до файла где хранится информация об узле
        out.write((char *) &filePath, sizeof filePath.size());
        int sizeParent = fileParent.size();
        out.write((char *) &sizeParent, sizeof sizeParent);
        // записываем путь до родителя
        for (char &i : fileParent) {
            out.write((char *) (&i), sizeof(char));
        }
        //Закрываем файл
        out.close();
    }

    // деструктор
    ~Node() {
        clearInfo();
    }
};

class BTree {
private:
    // корень дерева
    Node *root;
public:
    BTree() {
        root = new Node("nil", 0);
    }

    Node *&getRoot() {
        return root;
    }

    /*
     * Поиск в BTree (node-узел где мы ищем , key - ключ который мы ищем)
     */
    string findValue(Node *node, int key) {
        // индекс
        int i = 0;
        vector<int> vectorKeys = node->getKeys();
        // находим место для вставки ключа
        while (i < node->getCountKeys() && key > vectorKeys[i]) {
            i += 1;
        }
        // если такой ключ уже существует возвращаем значение (первое условие для того чтобы не зайти при пустом vectorKeys так как там 0)
        if (i < node->getCountKeys() && key == vectorKeys[i]) {
            return to_string(node->getValues()[i]);
        } else if (node->getIsList()) {
            return "nil";
        }
            // должны считать узел i-ого поддерева ключа node
        else {
            string path = node->getChildren()[i];
            Node children(path);
            children.readFile(path);
            return findValue(&children, key);
        }

    };

    /*
     * Разбиение узла BTree
     */
    static void split(Node *x, int index) {
        Node z(x->getFilePath(), 0);
        // получаем от нового корня путь до первого ребенка который как раз переполнен
        string path = x->getChildren()[index];
        // листик который как раз переполнился
        Node y(path);
        // узел который был ребенком для переданного узла по i индексу
        y.readFile(path);
        // присвоение для новосозданного узла количества ключей
        z.setIsList(y.getIsList());
        z.setCountKeys(t - 1);
        // копируем ключи и значения в новый листик (здесь ключи которые справа от медианы)
        for (int i = 0; i < t - 1; ++i) {
            z.getKeys().push_back(y.getKeys()[i + t]);
            z.getValues().push_back(y.getValues()[i + t]);
            z.getChildren().emplace_back("nil");
        }
        // берем ключ и значение которые хотим вытащить из родителя
        int key = y.getKeys()[t - 1];
        int value = y.getValues()[t - 1];
        y.setCountKeys(t - 1);
        // удаляем из переполненного листика все после медианы и саму медиану (на самом деле можно не делать)
        // так как мы записываем по количеству ключей, а это записываем выше, но если бы фор не по нему был то это важно
        // далее так лучше не делать чтобы все работало быстрее
        for (int j = 0; j < t; ++j) {
            y.getKeys().pop_back();
            y.getValues().pop_back();
        }
        // если узел имеет детей добавляем в новосозданный узел ссылки на детей
        if (!y.getIsList()) {
            z.setIsList(false);
            for (int j = t - 1; j >= 0; --j) {
                z.getChildren()[j] = y.getChildren()[j + t];
                y.getChildren().pop_back();
            }
        }

        // переносим медиану на верх
        x->getKeys().push_back(key);
        x->setCountKeys(x->getCountKeys() + 1);
        x->getValues().push_back(value);
        x->getChildren().push_back(z.getFilePath());
        // Вставка ключа и значения по индексу
        for (int i = x->getCountKeys() - 1; i > index; i--) {
            // своеобразный буфер можно еще через XOR
            x->getKeys()[i - 1] += x->getKeys()[i];
            x->getKeys()[i] = x->getKeys()[i - 1] - x->getKeys()[i];
            x->getKeys()[i - 1] -= x->getKeys()[i];

            x->getValues()[i - 1] += x->getValues()[i];
            x->getValues()[i] = x->getValues()[i - 1] - x->getValues()[i];
            x->getValues()[i - 1] -= x->getValues()[i];

            string str = x->getChildren()[i + 1];
            x->getChildren()[i + 1] = x->getChildren()[i];
            x->getChildren()[i] = str;
        }
        // записываем все в файлы
        x->writeToFile(x->getFilePath());
        y.writeToFile(y.getFilePath());
        z.writeToFile(z.getFilePath());
    }

    /*
     * Вставка элемента в BTree
     */
    void insertElement(int key, int value) {
        if (findValue(root, key) == "nil") {
            Node *r = root;
            if (r->getCountKeys() == 2 * t - 1) {
                // создаем новый узел, он теперь корень
                Node *s = new Node("nil", 0);
                r->getFileParent() = s->getFilePath();
                root = s;
                s->setIsList(false);
                // не обязательно и так по дефолту 0
                s->setCountKeys(0);
                s->getChildren()[0] = r->getFilePath();
                // записываем корень в файл не знаю зачем
                r->writeToFile(r->getFilePath());
                split(s, 0);
                insertNonFull(s, key, value);
            } else insertNonFull(r, key, value);
        } else {
            result += "false\n";
            return;
        }
        result += "true\n";
    }

    /*
     * Добавление в незаполненный узел или листик
     */
    void insertNonFull(Node *x, int key, int value) {
        int index = x->getCountKeys();
        // x листик
        if (x->getIsList()) {
            x->getValues().push_back(value);
            x->getKeys().push_back(key);
            // сдвигаем вправо на 1 все ключи и значения пока не найдем место для
            // ключа и значения и записываем их
            while (index >= 1 && key < x->getKeys()[index - 1]) {
                x->getKeys()[index - 1] += x->getKeys()[index];
                x->getKeys()[index] = x->getKeys()[index - 1] - x->getKeys()[index];
                x->getKeys()[index - 1] -= x->getKeys()[index];
                x->getValues()[index - 1] += x->getValues()[index];
                x->getValues()[index] = x->getValues()[index - 1] - x->getValues()[index];
                x->getValues()[index - 1] -= x->getValues()[index];
                index -= 1;
            }
            x->setCountKeys(x->getCountKeys() + 1);
            x->writeToFile(x->getFilePath());
            // если x - узел
        } else {
            // поиск индекса для вставки в поддеревья
            while (index >= 1 && key < x->getKeys()[index - 1]) index--;
            Node y = Node();
            // путь до подходящего поддерева
            string path = x->getChildren()[index];
            y = Node(path);
            y.readFile(y.getFilePath());
            if (y.getCountKeys() == 2 * t - 1) {
                split(x, index);
                // Если ключ больше чем медиана, то кладем его созданную правую ноду относительно медианы
                if (key > x->getKeys()[index]) {
                    index++;
                    // Инициализируем правую ноду
                    y = Node(x->getChildren()[index]);
                }
                // чтение из файлы информации о ноде
                y.readFile(y.getFilePath());
            }
            // Вставляем в ноду y
            insertNonFull(&y, key, value);
        }

    }

    pair<string, int> Find(Node *node, int key) {
        node->readFile(node->getFilePath());
        // индекс
        int i = 0;
        vector<int> vectorKeys = node->getKeys();
        // находим место для вставки ключа
        while (i < node->getCountKeys() && key > vectorKeys[i]) {
            i += 1;
        }
        // если такой ключ уже существует возвращаем значение (первое условие для того чтобы не зайти при пустом vectorKeys так как там 0)
        if (i < node->getCountKeys() && key == vectorKeys[i]) {
            pair<string, int> pair = make_pair(node->getFilePath(), i);
            return pair;
        } else if (node->getIsList()) {
            pair<string, int> pair = make_pair("nil", 0);
            return pair;
        }
            // должны считать узел i-ого поддерева ключа node
        else {
            string path = node->getChildren()[i];
            Node children(path);
            children.readFile(path);
            return Find(&children, key);
        }
    }


    /*
     * Удаление ключа со значением из BTree
     */
    string removeElement(Node *node, int key) {

        int index = 0;
        while (index < node->getCountKeys() && node->getKeys()[index] < key)
            index++;
        // Ключ который хотим удалить находится в этой ноде
        if (index < node->getCountKeys() && node->getKeys()[index] == key) {
            // если нода листик
            if (node->getIsList()) {
                return removeElementInLeaf(node, index);
                // если нода-узел имеет детей
            } else {
                return removeElementInNonLeaf(node, index);
            }
        } else {
            // Если этот узел лист то ключа не нашлось
            if (node->getIsList()) {
                // может надо null
                return "nil";
            }
            // Флаг показывает есть ли ключ в корне поддерева
            bool flag = (index == node->getCountKeys());
            Node child(node->getChildren()[index]);
            child.readFile(child.getFilePath());
            // если в ребенке меньше t ключей и есть ссылки на другие узлы
            if (child.getCountKeys() < t && node->getChildren().size() != 1) {
                fill(node, index);
                node->readFile(node->getFilePath());
            }
            // Если последний дочерний элемент был объединен, он должен быть объединен с предыдущим
            if (flag && index > node->getCountKeys()) {
                child = Node(node->getChildren()[index - 1]);
                child.readFile(child.getFilePath());
                return removeElement(&child, key);
            } else {
                child = Node(node->getChildren()[index]);
                child.readFile(child.getFilePath());
                return removeElement(&child, key);
            }
        }
    }


    /*
     * Удаление ключа со значением из листика
     */
    static string removeElementInLeaf(Node *node, int index) {
        string finalValue = to_string(node->getValues()[index]);
        // Перемещение на одно место влево относительно индекса
        for (int i = index + 1; i < node->getCountKeys(); ++i) {
            node->getKeys()[i - 1] = node->getKeys()[i];
            node->getValues()[i - 1] = node->getValues()[i];
        }
        node->getKeys().pop_back();
        node->setCountKeys(node->getCountKeys() - 1);
        node->getValues().pop_back();
        node->writeToFile(node->getFilePath());
        return finalValue;
    }


    /*
     * Удаление ключа со значением из узла
     */
    string removeElementInNonLeaf(Node *node, int index) {
        int key = node->getKeys()[index];
        int value = node->getValues()[index];
        Node childLeft(node->getChildren()[index]);
        childLeft.readFile(childLeft.getFilePath());
        Node childRight(node->getChildren()[index + 1]);
        childRight.readFile(childRight.getFilePath());

        // если у левого соседа больше t-1 ключ забираем у него
        if (childLeft.getCountKeys() > t - 1) {
            vector<int> rVector = getRightKeyValue(node, index);
            node->getKeys()[index] = rVector[0];
            node->getValues()[index] = rVector[1];
            removeElement(&childLeft, rVector[0]);
            childLeft.writeToFile(childLeft.getFilePath());
            node->writeToFile(node->getFilePath());
        }
            // если у правого соседа больше t-1 ключ забираем у него
        else if (childRight.getCountKeys() > t - 1) {
            vector<int> lVector = getLeftKeyValue(node, index);
            node->getKeys()[index] = lVector[0];
            node->getValues()[index] = lVector[1];
            removeElement(&childRight, lVector[0]);
            childRight.writeToFile(childRight.getFilePath());
            node->writeToFile(node->getFilePath());
        }
            // если у двух соседей меньше или равно t-1 ключям то обьединяем с правым
        else {
            merge(node, index);
            childLeft.readFile(childLeft.getFilePath());
            removeElement(&childLeft, key);
            childLeft.writeToFile(childLeft.getFilePath());
            node->writeToFile(node->getFilePath());
        }
        return to_string(value);
    }

/*
 * Получение пары ключ-значение справа
 */
    static vector<int> getRightKeyValue(Node *node, int index) {
        // Двигаемся к правому узлу пока он не лист
        Node child(node->getChildren()[index]);
        child.readFile(child.getFilePath());
        while (!child.getIsList()) {
            Node secondChild(child.getChildren()[child.getCountKeys()]);
            secondChild.readFile(secondChild.getFilePath());
            child = secondChild;
        }
        vector<int> rVector = {child.getKeys()[child.getCountKeys() - 1], child.getValues()[child.getCountKeys() - 1]};
        return rVector;
    }

    /*
     * Получение пары ключ-значение слева
     */
    static vector<int> getLeftKeyValue(Node *node, int index) {
        // Продолжаем двигаться к самому правому узлу, пока не достигнем листа
        Node child(node->getChildren()[index + 1]);
        child.readFile(child.getFilePath());
        while (!child.getIsList()) {
            Node secondChild(child.getChildren()[0]);
            secondChild.readFile(secondChild.getFilePath());
            child = secondChild;
        }
        vector<int> lVector = {child.getKeys()[0], child.getValues()[0]};
        return lVector;
    }

    /*
     * Заполняем ребенка
     */
    static void fill(Node *node, int index) {
        Node childLeft;
        Node childRight;
        if (index != 0) {
            childLeft = Node(node->getChildren()[index - 1]);
            childLeft.readFile(childLeft.getFilePath());
        }
        if (index != node->getCountKeys()) {
            childRight = Node(node->getChildren()[index + 1]);
            childRight.readFile(childRight.getFilePath());
        }
        // если ребенок не самый левый и у левого соседа > t -1 ключей
        if (index != 0 && childLeft.getCountKeys() > t - 1) {
            borrowLeftElement(node, index);
        }
            // если ребенок не самый правый и у правого соседа > t -1 ключей
        else if (index != node->getCountKeys() && childRight.getCountKeys() > t - 1) {
            borrowRightElement(node, index);
        }
            // если у обоих соседей <= t -1 ключей
        else {
            if (index != node->getCountKeys())
                merge(node, index);
            else
                merge(node, index - 1);
        }
    }


    /*
     * Берем ключ у соседа слева через родителя
     */
    static void borrowLeftElement(Node *node, int index) {

        Node childLeft(node->getChildren()[index - 1]);
        childLeft.readFile(childLeft.getFilePath());
        Node childRight(node->getChildren()[index]);
        childRight.readFile(childRight.getFilePath());
        childRight.getKeys().push_back(0);
        childRight.getValues().push_back(0);
        for (int i = childRight.getCountKeys() - 1; i >= 0; --i) {
            childRight.getKeys()[i + 1] = childRight.getKeys()[i];
            childRight.getValues()[i + 1] = childRight.getValues()[i];
        }
        if (!childRight.getIsList()) {
            childRight.getChildren().emplace_back("nil");
            for (int i = childRight.getCountKeys(); i >= 0; --i) {
                childRight.getChildren()[i + 1] = childRight.getChildren()[i];
            }
        }
        // Установка первого ключа ребенка равным ключом [index-1] от текущего узла
        childRight.getKeys()[0] = node->getKeys()[index - 1];
        childRight.getValues()[0] = node->getValues()[index - 1];
        // Перемещение последнего потомка брата как первого потомка
        if (!childRight.getIsList()) {
            childRight.getChildren()[0] = childLeft.getChildren()[childLeft.getCountKeys()];
            childLeft.getChildren().pop_back();
        }
        // Перемещение ключа от родного брата к родителю
        node->getKeys()[index - 1] = childLeft.getKeys()[childLeft.getCountKeys() - 1];
        node->getValues()[index - 1] = childLeft.getValues()[childLeft.getCountKeys() - 1];
        childRight.setCountKeys(childRight.getCountKeys() + 1);
        childLeft.setCountKeys(childLeft.getCountKeys() - 1);
        node->writeToFile(node->getFilePath());
        childRight.writeToFile(childRight.getFilePath());
        childLeft.writeToFile(childLeft.getFilePath());
    }

    /*
     * Берем ключ у соседа справа через родителя
     */
    static void borrowRightElement(Node *node, int index) {
        Node childRight(node->getChildren()[index + 1]);
        childRight.readFile(childRight.getFilePath());
        Node childLeft(node->getChildren()[index]);
        childLeft.readFile(childLeft.getFilePath());
        childLeft.getKeys().push_back(node->getKeys()[index]);
        childLeft.getValues().push_back(node->getValues()[index]);
        // если не листик забираем первую ссылку на ребенка у соседа справа
        if (!childLeft.getIsList()) {
            childLeft.getChildren().push_back(childRight.getChildren()[0]);
        }
        // ключ и значение родителю
        node->getKeys()[index] = childRight.getKeys()[0];
        node->getValues()[index] = childRight.getValues()[0];
        // Перемещение всех ключей и значений налево
        for (int i = 1; i < childRight.getCountKeys(); ++i) {
            childRight.getKeys()[i - 1] = childRight.getKeys()[i];
            childRight.getValues()[i - 1] = childRight.getValues()[i];
        }
        // Перемещение дочерних указателей налево
        if (!childRight.getIsList()) {
            for (int i = 1; i < childRight.getCountKeys() + 1; ++i) {
                childRight.getChildren()[i - 1] = childRight.getChildren()[i];
            }
            childRight.getChildren().pop_back();
        }
        childRight.setCountKeys(childRight.getCountKeys() - 1);
        childLeft.setCountKeys(childLeft.getCountKeys() + 1);
        node->writeToFile(node->getFilePath());
        childRight.writeToFile(childRight.getFilePath());
        childLeft.writeToFile(childLeft.getFilePath());
    }

    /*
     * Обьединение левой ноды с правой
     */
    static void merge(Node *node, int index) {
        Node childLeft(node->getChildren()[index]);
        childLeft.readFile(childLeft.getFilePath());
        Node childRight(node->getChildren()[index + 1]);
        childRight.readFile(childRight.getFilePath());
        // медианный ключ и значение
        childLeft.getKeys().push_back(node->getKeys()[index]);
        childLeft.getValues().push_back(node->getValues()[index]);
        // Копирование ключей из правой ноды
        for (int i = 0; i < childRight.getCountKeys(); ++i) {
            childLeft.getKeys().push_back(childRight.getKeys()[i]);
            childLeft.getValues().push_back(childRight.getValues()[i]);
        }
        // Когда нода не листик забираем ссылки на детей
        if (!childLeft.getIsList()) {
            for (int i = 0; i <= childRight.getCountKeys(); ++i) {
                childLeft.getChildren().push_back(childRight.getChildren()[i]);
            }
        }
        // Перемещаем ключи со значениями налево
        for (int i = index + 1; i < node->getCountKeys(); ++i) {
            node->getKeys()[i - 1] = node->getKeys()[i];
            node->getValues()[i - 1] = node->getValues()[i];
        }
        // Теперь перемещаем ссылки на детей
        for (int i = index + 2; i <= node->getCountKeys(); ++i) {
            node->getChildren()[i - 1] = node->getChildren()[i];
        }
        childLeft.setCountKeys(2 * t - 1);
        node->getKeys().pop_back();
        node->setCountKeys(node->getCountKeys() - 1);
        node->getValues().pop_back();
        node->getChildren().pop_back();
        node->writeToFile(node->getFilePath());
        childLeft.writeToFile(childLeft.getFilePath());
    }

    ~BTree() = default;
};

int main(int argc, char *argv[]) {
    t = stoi(argv[1]);
    if (t < 2) {
        cout << "Параметр t должен быть >= 2";
        return 1;
    }
    defaultPath = argv[2];
    defaultPath += "/";
    string pathInput = argv[3];
    string pathOutput = argv[4];
    BTree tree = BTree();
    fstream file(pathInput);
    string command;
    int key, value;
    // чтение файла
    while (!file.eof()) {
        file >> command;
        if (command == "insert") {
            file >> key >> value;
            tree.insertElement(key, value);
        }
        if (command == "find") {
            file >> key;
            string resFind = tree.findValue(tree.getRoot(), key);
            if (resFind == "nil") {
                result += "null\n";
            } else result += resFind + "\n";
        }
        if (command == "delete") {
            file >> key;
            string resFind = tree.removeElement(tree.getRoot(), key);
            if (resFind == "nil") {
                result += "null\n";
            } else result += resFind + "\n";

        }
    }
    result.pop_back();
    // поток для записи
    std::ofstream out;
    // открываем файл для записи
    out.open(pathOutput);
    if (out.is_open()) {
        out << result;
    }
    file.close();
    result.clear();
    tree.getRoot()->clearInfo();
    return 0;
}

