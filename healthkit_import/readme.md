# Healthkit extract instructions

Use the [health-to-sqlite project][1] to import any healthkit data. This assumes access to the `export.zip` file created by the Apple Health app.

## Steps

install: `pip install healthkit-to-sqlite` into a modern Python 3 environment (tested with 3.10).

Then load the data into a sqlite file using `healthkit-to-sqlite export.zip user.db`.

Then update the data using `sqlite3` cli-client: `sqlite3 user.db < post_import.sql`. Note that currently the version number of `healthkit-to-sqlite` is hardcoded in the script.

**WARNING**: This code currently doesn't handle repeated imports.

## TODO:
    * Handle repeated importing of the data into a single sqlite file
    * Update the healthkit-to-sqlite version automatically

[1]: https://github.com/dogsheep/healthkit-to-sqlite
