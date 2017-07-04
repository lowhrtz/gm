#include "PDFCreator.h"
#include <QString>

/*"<p class=page-break></p>"*/

PDFCreator::PDFCreator(QString markup) {
    /*QString html =
    "<style type=text/css>"
    ".border {"
    "color: red;"
    "border-style: solid;"
    "border-color: purple;"
    "}"
    ".bigger-font {"
    "font-size: 15px;"
    "}"
    ".pad-cell {"
    "padding-left: 5px;"
    "padding-right: 5px;"
    "}"
    ".pad-bottom {"
    "padding-bottom: 5px;"
    "}"
    ".no-pad {"
    "padding: 0px;"
    "}"
    ".float-right {"
    "float: right;"
    "}"
    "p.page-break {"
    "page-break-after:always;"
    "}"
    "</style>"
    "<h1 align=center>Character Name</h1>"
    "<img class=float-right align=right height=140 src=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAQAAAAEACAYAAABccqhmAAAFLklEQVR4nO3dMYscdRzG8eckRYpDRIIEi3B2EURSScqzEKzSmMJK7grxXQR8Ab6GiJ2NoNUVgRNsAjZphHQGQUllESxSCGeRswm3m53ZvZ2dPJ8PDFfMzv1/W8yX+y+7twkAAAAAAAAA8LrYe8X5q0k+2sYgA/yR5MnUQ8ACt5LcmXqIl3yXkffMQZKzHTvujXkisCVHmf4eefk4XDTsGxt5ysAsCQAUEwAodmXkdU82OcQS15Lsb2ktuCxPk5xsaa1Pk1xf9cFjA/DeyOuGup8XL6rAnD1OcryltU4zIAC2AFBs7F8AU3o/yZ9J/pp6ELjAO1MPMMQcA/D5+c93J50CLvbv1AMMYQsAxQQAigkAFBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBsjp8GTJKHSZ5PPQRc4HqSm1MPsao5BuBBks+SPJt6ELjAUV78J6tZmOMW4Je4+WEj5hgAYEMEAIoJABQTACgmAFBMAKDY2PcBnG50isVm84YKWOJ2kt+3tNbK3wqUjA/A4cjroNHVJAdTD3ERWwAoJgBQTACg2N4rzu8nubvi71rnAxAnSb5f8bGPzg/YRftJrg285tcR1/zvQZIvX/GYp9nCp2fP1ji+uezhYIf9lvH3zo/rLLzJLcCsvhUV8BoAVBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBMAKCYAEAxAYBiAgDFBACKCQAUEwAoJgBQTACgmABAMQGAYgIAxQQAigkAFBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBMAKCYAEAxAYBiAgDFBACKCQAUEwAoJgBQTACgmABAMQGAYgIAxQQAigkAFBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBMAKCYAEAxAYBiAgDFBACKCQAUEwAoJgBQTACgmABAMQGAYgIAxQQAigkAFBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBMAKCYAEAxAYBiAgDFBACKCQAUEwAoJgBQTACgmABAMQGAYgIAxQQAigkAFBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBMAKCYAEAxAYBiAgDFBACKCQAUEwAoJgBQTACgmABAMQGAYgIAxQQAigkAFBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBMAKCYAEAxAYBiAgDFBACKCQAUEwAoJgBQTACgmABAMQGAYgIAxQQAigkAFBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBMAKCYAEAxAYBiAgDFBACKCQAUEwAodmXJuf0kbw/4XevE5M0kN1Z43N9J/lljHbhMt5J8OOK6t9ZY80aSL5ac/yEj75mjJGc7dhyNeSKwJfcy/T3y8nGwbGBbACgmAFBMAKDYshcBgfU9TfJ4S2sdDr1gaAB+TvLx0EVGOs2IJwQ75iTJ8ZbWOht6gS0AFBMAKCYAUEwAoJgAQDEBgGJzex/AV0k+mXoIWOCDqQcYam4BuH1+ABtgCwDFBACKCQAUEwAoJgBQTACgmABAMQGAYgIAxQQAigkAFBMAKCYAUGxunwZMXvybZdg1++fHrMwtAMdJvp16CFjgXpKvpx5iCFsAKCYAUGzoFuBmkvuXMciCtWDu7maHv+FqaACuJzm6hDngdbXTLw7aAkAxAYBiAgDF9pacu5XkzrYGWdFPSR5NPQQscHB+7JKHSZ5PPQQAAAAAAAAAcPn+A+VH6ACOcfiQAAAAAElFTkSuQmCC></img>"
    "<table><tr><td>"
    ""
    ""
    "<table class='border bigger-font' border=1 ><tr><td>"
    "<table class=pad-cell>"
    "<tr><td align=right class=pad-cell>Str:</td><td align=right class=pad-cell>9</td><td style=font-size:8px;vertical-align:middle;> +0 to hit, +0 damage</td></tr>"
    "<tr><td align=right class=pad-cell>Int:</td><td align=right class=pad-cell>10</td></tr>"
    "<tr><td align=right class=pad-cell>Wis:</td><td align=right class=pad-cell>12</td></tr>"
    "<tr><td align=right class=pad-cell>Dex:</td><td align=right class=pad-cell>8</td></tr>"
    "<tr><td align=right class=pad-cell>Con:</td><td align=right class=pad-cell>7</td></tr>"
    "<tr><td align=right class=pad-cell>Cha:</td><td align=right class=pad-cell>14</td></tr>"
    "</table>"
    "</td></tr></table></td>"
    ""
    "<td class=pad-cell><table class=no-pad height=absolute>"
    "<tr><td class=pad-bottom><b>Name: </b></td><td align=right>Character Name</td></tr>"
    "<tr><td class=pad-bottom><b>Gender: </b></td><td align=right>Character Gender</td></tr>"
    "<tr><td class=pad-bottom><b>Alignment: </b></td><td align=right>Character Alignment</td></tr>"
    "<tr><td class=pad-bottom><b>Class: </b></td><td align=right>Character Class</td></tr>"
    "<tr><td class=pad-bottom><b>Race: </b></td><td align=right>Character Race</td></tr>"
    "</table></td>"
    ""
    ""
    ""
    "</td></tr></table>"
    "<hr />";*/

//    QTextDocument document;
    QFont font("Times", 10, QFont::Normal);
    document.setDefaultFont(font);
//    document.setHtml(html);
    document.setHtml(markup);

//    QPrinter printer(QPrinter::PrinterResolution);
//    printer.setResolution(QPrinter::PrinterResolution);
    printer = new QPrinter(QPrinter::PrinterResolution);
    printer->setOutputFormat(QPrinter::PdfFormat);
    printer->setPaperSize(QPrinter::A4);
    printer->setOutputFileName("/tmp/test.pdf");
    printer->setPageMargins(QMarginsF(15, 15, 15, 15));


    document.setPageSize(printer->pageRect().size()); //This disables printing the page number

//    document.print(&printer);
}

void PDFCreator::save() {
    document.print(printer);
}

/*"<img src=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAQAAAAEACAYAAABccqhmAAAFLklEQVR4nO3dMYscdRzG8eckRYpDRIIEi3B2EURSScqzEKzSmMJK7grxXQR8Ab6GiJ2NoNUVgRNsAjZphHQGQUllESxSCGeRswm3m53ZvZ2dPJ8PDFfMzv1/W8yX+y+7twkAAAAAAAAA8LrYe8X5q0k+2sYgA/yR5MnUQ8ACt5LcmXqIl3yXkffMQZKzHTvujXkisCVHmf4eefk4XDTsGxt5ysAsCQAUEwAodmXkdU82OcQS15Lsb2ktuCxPk5xsaa1Pk1xf9cFjA/DeyOuGup8XL6rAnD1OcryltU4zIAC2AFBs7F8AU3o/yZ9J/pp6ELjAO1MPMMQcA/D5+c93J50CLvbv1AMMYQsAxQQAigkAFBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBsjp8GTJKHSZ5PPQRc4HqSm1MPsao5BuBBks+SPJt6ELjAUV78J6tZmOMW4Je4+WEj5hgAYEMEAIoJABQTACgmAFBMAKDY2PcBnG50isVm84YKWOJ2kt+3tNbK3wqUjA/A4cjroNHVJAdTD3ERWwAoJgBQTACg2N4rzu8nubvi71rnAxAnSb5f8bGPzg/YRftJrg285tcR1/zvQZIvX/GYp9nCp2fP1ji+uezhYIf9lvH3zo/rLLzJLcCsvhUV8BoAVBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBMAKCYAEAxAYBiAgDFBACKCQAUEwAoJgBQTACgmABAMQGAYgIAxQQAigkAFBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBMAKCYAEAxAYBiAgDFBACKCQAUEwAoJgBQTACgmABAMQGAYgIAxQQAigkAFBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBMAKCYAEAxAYBiAgDFBACKCQAUEwAoJgBQTACgmABAMQGAYgIAxQQAigkAFBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBMAKCYAEAxAYBiAgDFBACKCQAUEwAoJgBQTACgmABAMQGAYgIAxQQAigkAFBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBMAKCYAEAxAYBiAgDFBACKCQAUEwAoJgBQTACgmABAMQGAYgIAxQQAigkAFBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBMAKCYAEAxAYBiAgDFBACKCQAUEwAoJgBQTACgmABAMQGAYgIAxQQAigkAFBMAKCYAUEwAoJgAQDEBgGICAMUEAIoJABQTACgmAFBMAKCYAEAxAYBiAgDFBACKCQAUEwAodmXJuf0kbw/4XevE5M0kN1Z43N9J/lljHbhMt5J8OOK6t9ZY80aSL5ac/yEj75mjJGc7dhyNeSKwJfcy/T3y8nGwbGBbACgmAFBMAKDYshcBgfU9TfJ4S2sdDr1gaAB+TvLx0EVGOs2IJwQ75iTJ8ZbWOht6gS0AFBMAKCYAUEwAoJgAQDEBgGJzex/AV0k+mXoIWOCDqQcYam4BuH1+ABtgCwDFBACKCQAUEwAoJgBQTACgmABAMQGAYgIAxQQAigkAFBMAKCYAUGxunwZMXvybZdg1++fHrMwtAMdJvp16CFjgXpKvpx5iCFsAKCYAUGzoFuBmkvuXMciCtWDu7maHv+FqaACuJzm6hDngdbXTLw7aAkAxAYBiAgDF9pacu5XkzrYGWdFPSR5NPQQscHB+7JKHSZ5PPQQAAAAAAAAAcPn+A+VH6ACOcfiQAAAAAElFTkSuQmCC></img>"*/
