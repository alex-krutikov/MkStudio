#include "misc.h"

#include <QRegularExpression>

//#######################################################################################
//
//#######################################################################################
ScriptHighlighter::ScriptHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;
    QStringList keywordPatterns;

    QTextCharFormat cf1;
    QTextCharFormat keywordFormat;
    QTextCharFormat singleLineCommentFormat;

    keywordPatterns << "enum"
                    << "bitmask"
                    << "tooltip";

    // =1:
    cf1.setForeground(Qt::darkBlue);
    cf1.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("=-?\\d+:");
    rule.format = cf1;
    highlightingRules << rule;

    // служебные слова
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    foreach (QString pattern, keywordPatterns)
    {
        rule.pattern = QRegularExpression("\\b" + pattern + "\\b");
        rule.format = keywordFormat;
        highlightingRules << rule;
    }

    // однострочные комментарии
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules << rule;
}

//==============================================================================
//
//==============================================================================
void ScriptHighlighter::highlightBlock(const QString &text)
{
    int index = 0;

    for (const HighlightingRule &rule : highlightingRules)
    {
        QRegularExpression expression(rule.pattern);
        QRegularExpressionMatchIterator matches = expression.globalMatch(text);

        while (matches.hasNext())
        {
            QRegularExpressionMatch match = matches.next();
            int length = match.capturedLength();
            setFormat(match.capturedStart(), length, rule.format);
        }
    }
}
