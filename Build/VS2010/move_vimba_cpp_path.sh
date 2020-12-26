#!/bin/bash

vimba_path='C:\Users\Public\Documents\Allied Vision\Vimba_4.0\VimbaCPP_Examples\'
vimba_path2='C:/Users/Public/Documents/Allied Vision/Vimba_4.0/VimbaCPP_Examples/'

vimba_example_path=`find /mnt/c/Users/Public  -type d -name "VimbaCPP_Examples" | head -n 1`
echo $vimba_example_path

source_str='..\..\..\..\Build\VS2010\'
echo $source_str
echo $vimba_path

# sed --debug -i  "s/'$source_str'/'$vimba_path'\Build\VS2010\/g" AsynchronousGrabMFC2.sln

awk '{sub(/\.\.\\\.\.\\\.\.\\\.\./,"C:\\Users\\Public\\Documents\\Allied Vision\\Vimba_4.0\\VimbaCPP_Examples")}1' AsynchronousGrabConsole.vcxproj > temp1.vcxproj
# awk '{sub(/\.\.\\\.\.\\\.\.\\\.\./, '$vimba_path2')}1' AsynchronousGrabMFC3.vcxproj > temp1.vcxproj
cat temp1.vcxproj

# awk '{sub(/\.\.\\\.\.\\Source;\.\.\\\.\.\\\.\.\\\.\.\\\.\.;/, "\.\.\\\.\.\\Source;C:\\Users\\Public\\Documents\\Allied Vision\\Vimba_4.0\\VimbaCPP_Examples\\;")}1' temp1.vcxproj > temp2.vcxproj
# cat temp2.vcxproj
cp temp1.vcxproj AsynchronousGrabConsole.vcxproj


