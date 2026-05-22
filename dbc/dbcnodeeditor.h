#ifndef DBCNODEEDITOR_H
#define DBCNODEEDITOR_H

#include <QDialog>
#include "dbc_classes.h"
#include "dbchandler.h"

namespace Ui {
class DBCNodeEditor;
}

class DBCNodeEditor : public QDialog
{
    Q_OBJECT

public:
    explicit DBCNodeEditor(QWidget *parent = nullptr);
    ~DBCNodeEditor();
    void showEvent(QShowEvent*) override;
    void setNodeRef(DBC_NODE *node);
    void setFileIdx(int idx);
    void refreshView();

signals:
    void updatedTreeInfo(DBC_NODE *node);

private:
    Ui::DBCNodeEditor *ui;

    DBCHandler *dbcHandler;
    DBC_NODE *dbcNode;
    DBCFile *dbcFile;

    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void readSettings();
    void writeSettings();
    void generateSampleText();
};

#endif // DBCNODEEDITOR_H
