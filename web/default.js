/* Top logo */
function TopLogo()
{
  document.write("<table border=0 width=\"100%\" cellpadding=0 cellspacing=0>");
  document.write("  <tbody>");
  document.write("    <tr>");
  document.write("      <td width=639 align=\"left\">");
  document.write("        <a href=\"http://flylegacy.sourceforge.net/\">");
  document.write("          <img src=\"images/logo.jpg\" border=0 alt=\"\">");
  document.write("        </a>");
  document.write("      </td>");
  document.write("      <td align=\"left\">");
  document.write("        <a href=\"http://flylegacy.sourceforge.net/\">");
  document.write("          <img src=\"images/logo_edge.jpg\" border=0 alt=\"\">");
  document.write("        </a>");
  document.write("      </td>");
  document.write("    </tr>");
  document.write("  </tbody>");
  document.write("</table>");
}

/* Left-column menu */
function LeftMenu()
{
  document.write("<h3 class=\"left_menu_header\">Main</h3>");
  document.write("<a href=\"index.html\" class=left_menu_item>Home</a><br>");
  document.write("<a href=\"introduction.html\" class=left_menu_item>Introduction</a><br>");
  document.write("<a href=\"status.html\" class=left_menu_item>Status</a><br>");
/*   document.write("<a href=\"features.html\" class=left_menu_item>Features</a><br>"); */
  document.write("<a href=\"screenshots.html\" class=left_menu_item>Screenshots</a><br>");
  document.write("<h3 class=\"left_menu_header\">Support</h3>");
  document.write("<a href=\"docs.html\" class=left_menu_item>Documentation</a><br>");
  document.write("<a href=\"doxygen/\" class=left_menu_item>Doxygen</a><br>");
  document.write("<a href=\"wiki.html\" class=left_menu_item>Fly! Wiki</a><br>");
  document.write("<a href=\"faq.html\" class=left_menu_item>FAQ</a><br>");
  document.write("<h3 class=\"left_menu_header\">Links</h3>");
  document.write("<a href=\"http://www.sf.net/projects/flylegacy\" class=left_menu_item>SF.net Project</a><br>");
  document.write("<h3 class=\"left_menu_header\">Contribute</h3>");
  document.write("<a href=\"contributing.html\" class=left_menu_item>Contributing</a><br>");
  document.write("<a href=\"goals.html\" class=left_menu_item>Goals</a><br>");
}

/* Page footer */
function Footer()
{
  document.write("<table width=\"100%\">");
  document.write("  <tr width=\"100%\">");
  document.write("    <td colspan=\"3\"><hr></td>");
  document.write("  </tr>");
  document.write("  <tr width=\"100%\">");
  /* First column, SF logo */
  document.write("    <td align=\"left\" valign=\"center\">");
  document.write("      <a href=\"http://sourceforge.net\"><img src=\"http://sourceforge.net/sflogo.php?group_id=145467&amp;type=2\" width=\"125\" height=\"37\" border=\"0\" alt=\"SourceForge.net Logo\"/></a>");
  document.write("    </td>");
  /* Second column, copyright info */
  document.write("    <td align=\"center\" valign=\"center\">");
  document.write("      <p class=\"copyright\">(c)2005 Chris Wallace");
  document.write("    </td>");
  /* Third column, last update date/time */
  document.write("    <td align=\"right\" valign=\"center\">");
  document.write("      <p class=\"copyright\">Last Updated : 2005-Sep-17");
  document.write("    </td>");
  document.write("  </tr>");
  document.write("</table>");
}
