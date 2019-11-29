from django.db import models
from django.utils import timezone

# 기본적인 데이터베이스 구조입니다. 파라미터에 변경이 필요할 경우 깃에 커밋하실때 코멘트 남겨주시기바랍니다.
# fresh/die/dry/humid/lacksun/etc
class Data(models.Model):
    seq = models.AutoField(primary_key=True,blank=True)
    plant_name = models.CharField(max_length=200,null=True,blank=True)
    plant_picture = models.ImageField(blank=True, null=True)
    temp = models.CharField(max_length=128,null=True,blank=True)
    humid = models.CharField(max_length=128,null=True,blank=True)
    light = models.CharField(max_length=128,null=True,blank=True)
    flow_water = models.CharField(max_length=128,null=True,blank=True)
    ph_water = models.CharField(max_length=128,null=True,blank=True)
    tds_water = models.CharField(max_length=128,null=True,blank=True)
    started_date = models.DateTimeField(default=timezone.now)
    published_date = models.DateTimeField(blank=True, null=True)
    etc = models.CharField(max_length=128,null=True,blank=True)
    desc = models.TextField(null=True,blank=True)

    def __str__(self):
        return self.plant_name
