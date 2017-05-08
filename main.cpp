#include <QCoreApplication>
#include <QCommandLineParser>
#include <QStringList>
#include <QDebug>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>

#include "minimizer.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Boolean Function Minimizer");
    QCoreApplication::setApplicationVersion("v1.0");
    QCommandLineParser parser;

    parser.addPositionalArgument("[file1]", QCoreApplication::translate("main", "input file location."));
    parser.addPositionalArgument("[file2]", QCoreApplication::translate("main", "don't care file location."));

    QCommandLineOption echo(QStringList() << "e" << "echo",
                QCoreApplication::translate("main", "Echo process to torminal"));
    parser.addOption(echo);

    QCommandLineOption out(QStringList() << "o" << "output",
                QCoreApplication::translate("main", "Set output file name"));
    parser.addOption(out);

    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(app);

    const QStringList args = parser.positionalArguments();

    minimizer min;
    QObject::connect(&min, SIGNAL(quit()), &app, SLOT(quit()));

    if(args.size() > 2){
        cerr << parser.helpText().toStdString();
        return 0;
    }
    else {
        if(parser.isSet(echo))  min.setEcho();
        if(args.size() > 0) {
            min.setInName(args.at(0));
            if(args.size() == 2)    min.setDcName(args.at(1));
        }
        if(parser.isSet(out))   min.setOutName(parser.value(out));
    }

    if(!min.setInFile()) {
        cerr << "Error opening input file" << endl;
        return 1;
    }

    //if don't care file specified but could not open
    if(args.size() == 2 && !min.setDcFile()) {
        cerr << "Error opening don't care' file" << endl;
        return 1;
    }

    if(!min.setOutFile()) {
        cerr << "Error creating file" << endl;
        return 2;
    }

    cout << "Reading Files" << endl;
    min.read();

    return 0;
}
