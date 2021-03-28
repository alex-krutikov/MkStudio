#ifndef _MISC__H_
#define _MISC__H_

#include <QSyntaxHighlighter>

//==============================================================================
//
//==============================================================================
class ScriptHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    ScriptHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;
};

#endif
