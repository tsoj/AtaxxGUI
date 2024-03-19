#pragma once

#include <qtmetamacros.h>
#include <QFile>
#include <QPushButton>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>

class TextEditor : public QWidget {
    Q_OBJECT
   public:
    TextEditor(const std::string &file_path, QWidget *parent = nullptr);

   signals:
    void changed_settings();

   private slots:
    void save_file();

   private:
    QTextEdit *m_text_edit;
    QString m_file_path;
};
