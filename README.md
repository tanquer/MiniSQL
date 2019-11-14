# Introduction

This a simple SQL Database System.

# Compiler Requirements

- MSVC 14.0+

# Contents

It supports SQL commands as follows:

```SQL
create table tableName(
		attributeName type,
		attributeName type,
		attributeName type,
		...
		primary key (attributeName)
	);

	drop table tableName;

	create index indexName on tableName(attributeName);
	
	drop index indexName;
	
	select * from tableName;
	select * from tableName where condition1;
	select * from tableName where condition1 and condition2;
	condition represents that attribute op value, where op is > <> = >= <=.
	
	insert into tableName values(value1, value2);
	
	delete * from tableName;
	delete * from tableName where condition;
	
	quit;
	
	execfile file;  # exec the sql queries in the file.
```