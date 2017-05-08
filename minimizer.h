#ifndef MINIMIZER_H
#define MINIMIZER_H

#include <QObject>
#include <QString>
#include <fstream>
#include <string>
#include <vector>


class minimizer : public QObject
{
    Q_OBJECT
public:

    explicit minimizer(QObject *parent = 0);
    void setEcho();
    void setInName(QString name);
    void setInName(std::string name);
    void setDcName(QString name);
    void setDcName(std::string name);
    void setOutName(QString name);
    void setOutName(std::string name);
    bool setInFile();
    bool setDcFile();
    bool setOutFile();
    //process starting function
    void read();

private:
    bool echo;
    std::string inName;
    std::string dcName;
    std::string outName;
    std::ifstream inFile;
    std::ifstream dcFile;
    std::ofstream outFile;

    std::string final;
    std::vector<int> minterms;
    std::vector<int> dcterms;
    std::vector<std::string> binMinterms;
    std::vector<std::string> binDcterms;
    std::vector<std::vector<std::vector<std::string> > > implicants;
    std::vector<std::string> required;
    std::vector<std::string> essential;


    unsigned int n;     //number of inputs
    int max;            //minterm maximun value

    bool isMinterm(int t);
    bool isDcterm(int t);
    bool isEssential(std::string t);
    void minimize();
    void group();
    std::vector<std::vector<std::string> > group(std::vector<std::string> &v);
    int countOnes(std::string num);
    int combine(std::vector<std::vector<std::string> > &sizen);
    int compare(std::string n1, std::string n2, int &idx);
    std::vector<std::vector<std::string> > primeChart();
    std::vector<std::vector<std::string> > reduce(std::vector<std::vector<std::string> > &chart);
    void findRequired(std::vector<std::vector<std::string> > &chart);
    void formFinal();
    bool contains(std::string imp, std::string mint);
    std::string toA(std::string imp);
    void printRound(int r);
    void printChart(std::vector<std::vector<std::string> > chart);

    static std::vector<std::vector<std::string> > const terms;
    static std::vector<std::vector<std::string> > const binterms;
    static std::vector<std::string> const F;
    static std::vector<std::string> const D;

signals:
    void quit();
};

#endif // MINIMIZER_H
