{% test non_negative_num(model, column_name) %}

with validation as (

    select
        {{ column_name }} as non_neg_field

    from {{ model }}

),

validation_errors as (

    select
        non_neg_field

    from validation
    -- if this is true, then even_field is actually odd!
    where non_neg_field < 0
)

select *
from validation_errors

{% endtest %}
