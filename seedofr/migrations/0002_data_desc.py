# Generated by Django 2.0 on 2019-11-29 08:50

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('seedofr', '0001_initial'),
    ]

    operations = [
        migrations.AddField(
            model_name='data',
            name='desc',
            field=models.TextField(blank=True, null=True),
        ),
    ]