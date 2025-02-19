   find /home/rob/Documents/bGeigieZen\ org/bGeigieZen-master/bgeigiezen_firmware/ -type f -name "*.c" | while read -r c_file                                     
   do                                                                                                                                                             
     c_included="`grep  -r --include "*.[ch]" -e '#include' "$c_file"`"                                                                                           
     echo "$c_included"                                                                                                                                           
   done | dot -Tpdf > firmware_flowchart.pdf  