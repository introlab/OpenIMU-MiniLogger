#ifndef QURLVALIDATOR_H
#define QURLVALIDATOR_H

//file taken from  : https://github.com/Skycoder42/QUrlValidator

#include <QObject>
#include <QValidator>


class  QUrlValidator : public QValidator
{
    Q_OBJECT

    Q_PROPERTY(QStringList allowedSchemes READ allowedSchemes WRITE setAllowedSchemes)

public:
    QUrlValidator(QStringList allowedSchemes, QObject *parent = nullptr);
    QUrlValidator(QObject *parent = nullptr);

    // QValidator interface
    State validate(QString &input, int &pos) const override;
    void fixup(QString &text) const override;
    QStringList allowedSchemes() const;

public slots:
    void setAllowedSchemes(QStringList allowedSchemes);

private:
    QStringList schemes;
    QRegularExpressionValidator *schemeValidator;
};


#endif // QURLVALIDATOR_H
