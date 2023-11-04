# Lazydog DBT project

This is a small [dbt-core]((https://docs.getdbt.com/docs/introduction)) project for
extracting, transforming / cleaning and in general preparing the data for further use.

## Setup

TODO: explain how to set up dbt

Sqlite support works through a plugin. What would be a schema in a different dbms is a separate file when using this plugin. Thus, all sources are linked to separate schemas/files. These need to be present in the dbt configuration.

Try running the following commands:
- dbt run
- dbt test
