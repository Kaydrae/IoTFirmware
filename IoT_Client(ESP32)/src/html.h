#ifndef HTML
#define HTML

// HTML for server
const char* html_data = 
    "<!DOCTYPE html>"

    "<html>"
        "<head>"
            "<title>WiFi Config</title>"
            "<meta charset=\"UTF-8\">"
        "</head>"
        "<body>"
            "<h1>Setup Your WiFi Connection</h1>"
            "<form action=\"/\" method=\"post\">"
                "<p>"
                    "<label>SSID:&nbsp;</label>"
                    "<input maxlength=\"32\" name=\"ssid\">"
                    "<br>"
                    "<label>PASSWD:&nbsp;</label>"
                    "<input maxlength=\"32\" name=\"passwd\">"
                    "<br>"
                    "<label>HOST:&nbsp;</label>"
                    "<input maxlength=\"64\" name=\"host\">"
                    "<br>"
                    "<label>PORT:&nbsp;</label>"
                    "<input maxlength=\"32\" name=\"port\">"
                    "<br>"
                    "<label>UUID:&nbsp;</label>"
                    "<input maxlength=\"36\" name=\"uuid\">"
                    "<br>"
                    "<label>DATA:&nbsp;</label>"
                    "<input maxlength=\"256\" name=\"data\">"
                    "<br>"
                    "<input type=\"submit\" value=\"send\">"
                    "<input type=\"reset\">"
                "</p>"
            "</form>"
        "</body>"
    "</html>";

#endif // HTML