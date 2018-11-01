#!/bin/bash


#
#     Copyright 2008 Nicola Baldo
#
#     This program is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
#
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with this program.  If not, see <http://www.gnu.org/licenses/>.
#


if test $# -lt 1 ; then
    echo "usage: $0 path [heading]"
    exit 1
fi

THUMBPATH="$1"

if test ! -d $THUMBPATH ; then
    echo "$THUMBPATH is not a directory"
    exit 1
fi

if test $# -lt 2 ; then
    HEADING="Plot Thumbnails"
else
    HEADING="$2"
fi

cat > "${THUMBPATH}/index.html" <<EOF
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">

<head>
  <title>Thumbnails</title>
  <meta name="GENERATOR" content="Quanta Plus" />
  <meta name="AUTHOR" content="Nicola Baldo" />
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />

 <style type="text/css">

body   { 
	font-family: Sans-serif; 
	font-size: 11px;
}

h1 {
        text-align: center;
	font-family: Sans-serif; 
	font-size: 24px;
}

div.plot {	
	float:left;
        
	margin-right: 0.5cm;
	margin-left:  0.5cm;
	margin-top:   0.5cm;
	margin-bottom:0.5cm;

        width: 200px;
}

a.resource {
	float:left;
        margin-right: 0.25cm;
}




 </style>
</head>
<body>

<h1 class="title"> $HEADING </h1>

EOF

cat ${THUMBPATH}/*.thumb >>  "${THUMBPATH}/index.html"


cat >> "${THUMBPATH}/index.html" <<EOF

  
</body>
</html>


EOF

