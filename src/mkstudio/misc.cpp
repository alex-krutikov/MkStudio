#include "misc.h"

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

    keywordPatterns << "enum" << "bitmask" << "tooltip";

    // =1:
    cf1.setForeground(Qt::darkBlue);
    cf1.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("=-?\\d+:");
    rule.format = cf1;
    highlightingRules << rule;

    // служебные слова
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    foreach (QString pattern, keywordPatterns)
    { rule.pattern = QRegExp("\\b"+pattern+"\\b");
      rule.format = keywordFormat;
      highlightingRules << rule;
    }

    // однострочные комментарии
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules << rule;
}

//==============================================================================
//
//==============================================================================
void ScriptHighlighter::highlightBlock(const QString &text)
{
  int index,length;

  foreach( HighlightingRule rule, highlightingRules )
  { QRegExp expression(rule.pattern);
    index = text.indexOf(expression);
    while (index >= 0)
    { length = expression.matchedLength();
      setFormat(index, length, rule.format);
      index = text.indexOf(expression, index + length);
    }
  }
}
