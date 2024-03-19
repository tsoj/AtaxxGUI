#include "texteditor.hpp"

TextEditor::TextEditor(const std::string &file_path, QWidget *parent)
    : QWidget(parent), m_file_path(file_path.c_str()) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_text_edit = new QTextEdit(this);
    QPushButton *save_button = new QPushButton("Save", this);

    layout->addWidget(m_text_edit);
    layout->addWidget(save_button);

    connect(save_button, &QPushButton::clicked, this, &TextEditor::save_file);
    setLayout(layout);

    QFile file(m_file_path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        m_text_edit->setText(in.readAll());
        file.close();
    }
}

void TextEditor::save_file() {
    QFile file(m_file_path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << m_text_edit->toPlainText();
        file.close();
    }
    emit changed_settings();
    this->close();
}