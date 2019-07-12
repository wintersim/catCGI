#!/bin/bash

sqlite3 cat.db "CREATE TABLE Cat(Id INT, Name TEXT);"
sqlite3 cat.db "CREATE TABLE Clicks(Name TEXT, Clicks INT);"
sqlite3 cat.db "INSERT INTO Clicks VALUES('cat',0)"
