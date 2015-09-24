/*

ConsoleModel is courtesy of Harbour FileBrowser
https://github.com/karip/harbour-file-browser

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#ifndef CONSOLEMODEL_H
#define CONSOLEMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QProcess>

/**
 * @brief The ConsoleModel class holds a list of strings for a QML list model.
 */
class ConsoleModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList lines READ lines() WRITE setLines(QString) NOTIFY linesChanged())

public:
    explicit ConsoleModel(QObject *parent = 0);
    ~ConsoleModel();

    // methods needed by ListView
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;

    // property accessors
    QStringList lines() const { return m_lines; }
    void setLines(QStringList lines);
    void setLines(QString lines);

    void appendLine(QString line);

    Q_INVOKABLE bool executeCommand(QString command, QStringList arguments);

signals:
    void linesChanged();
    void processExited(int exitCode);

private slots:
    void readProcessChannels();
    void handleProcessFinish(int exitCode, QProcess::ExitStatus status);
    void handleProcessError(QProcess::ProcessError error);

private:
    QProcess *m_process;
    QStringList m_lines;
};

#endif // CONSOLEMODEL_H
