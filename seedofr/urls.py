from django.urls import path
from django.conf import settings
from django.conf.urls.static import static
from . import views

urlpatterns = [
    # main
    path('', views.index, name='index'),
    path('seedofr/new', views.post_new, name='post_new'),
    path('seedofr/compare', views.post_compare, name='post_compare'),
    path('seedofr/<int:seq>/', views.post_detail, name='post_detail'),
    path('seedofr/list', views.post_list, name='post_list'),
] + static(settings.MEDIA_URL, document_root=settings.MEDIA_ROOT)
