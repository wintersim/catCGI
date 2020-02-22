#!/bin/bash

sqlite3 cat.db "CREATE TABLE IF NOT EXISTS Cat(Id INTEGER PRIMARY KEY, Name TEXT);"
sqlite3 cat.db "CREATE TABLE IF NOT EXISTS Clicks(Name TEXT PRIMARY KEY, Clicks INT);"
sqlite3 cat.db "INSERT INTO Clicks VALUES('cat',0)"
