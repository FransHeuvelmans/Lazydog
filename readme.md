# Project Lazydog

Inspired by [dogsheep][1] (and [Heedy][2] and [qs_ledger][3]) a personal health datalake.

## Plan

I want to add integrations to as many tools as possible.
The focus lies on FOSS applications and applications which hava an easy to use "export data" option.

## Tools

**merge_sqlite**: Combine the extracted sqlite files into a single sqlite file which is easier to analyze and further process

**healthkit_import** (Apple Health): Steps to use the *healthkit-to-sqlite* tool for importing data in Lazydog (Sql-code also a part of transformations)

**paseo_import**: Transform and merge Paseo[11] data (C - meson project). Includes SQL code to transform the sqlite output from *Paseo* into Lazydog-style (Sql-code also a part of transformations)

**[openscale-to-sqlite][4]**: Load in [openscale data][5]. (Golang project)

**[google-fit-to-sqlite][6]**: Load in [Google Fit][7] (Takeout) data (Rust project)

**[fast-n-fitness-to-sqlite][8]**: Load in [Fast-n-fitness][9] data (Dlang project)

**transformations**: A [dbt-core][10]-project which shows example sql-processing of the datasources

**bangle_import**: Support importing bangle smartwatch data

## Overarching TODOs
* Handle migrations
* Handle batching
* Handle duplicate data loading


[1]: https://dogsheep.github.io/
[2]: https://heedy.org/
[3]: https://github.com/markwk/qs_ledger
[4]: https://fossil.hillman.dev/openscale-to-sqlite
[5]: https://github.com/oliexdev/openScale
[6]: https://fossil.hillman.dev/google-fit-to-sqlite
[7]: https://www.google.com/fit/
[8]: https://fossil.hillman.dev/fast-n-fitness-to-sqlite
[9]: https://f-droid.org/en/packages/com.easyfitness/
[10]: https://docs.getdbt.com/
[11]: https://f-droid.org/packages/ca.chancehorizon.paseo/