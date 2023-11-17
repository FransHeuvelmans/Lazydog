{% test positive_num(model, column_name) %}

with validation as (

    select
        {{ column_name }} as pos_field

    from {{ model }}

),

validation_errors as (

    select
        pos_field

    from validation
    -- if this is true, then even_field is actually odd!
    where pos_field <= 0
)

select *
from validation_errors

{% endtest %}
