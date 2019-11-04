#include "qurlvalidator.h"
#include <QRegularExpressionValidator>
#include <QUrl>

QUrlValidator::QUrlValidator(QStringList allowedSchemes, QObject *parent) :
    QUrlValidator(parent)
{
    setAllowedSchemes(allowedSchemes);
}

QUrlValidator::QUrlValidator(QObject *parent) :
    QValidator(parent),
    schemes(),
    schemeValidator(new QRegularExpressionValidator(this))

{}

QValidator::State QUrlValidator::validate(QString &input, int &pos) const
{
    if(!schemes.isEmpty()) {
        auto res = schemeValidator->validate(input, pos);
        if(res != Acceptable)
            return res;
    }

    auto url = QUrl::fromUserInput(input);
    if(url.isValid())
        return Acceptable;
    else
        return Intermediate;
}

void QUrlValidator::fixup(QString &text) const
{
    text = QUrl::fromUserInput(text).toString();
}

QStringList QUrlValidator::allowedSchemes() const
{
    return schemes;
}

void QUrlValidator::setAllowedSchemes(QStringList allowedSchemes)
{
    schemes = allowedSchemes;
    schemeValidator->setRegularExpression(QRegularExpression(QStringLiteral(R"__(^(?:%1)\:\/\/.*)__")
                                                             .arg(schemes.join(QLatin1Char('|')))));
}
