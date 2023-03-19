# Paseo extraction instructions

Paseo's export is already in an Sqlite format.

To make the data more in line with the rest of Lazydog using an `sqlite3` client do

```{sh}
sqlite3 user.db < transform_datums.sql
```