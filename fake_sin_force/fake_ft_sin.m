close all; clear all;clc
addpath(fullfile(cd, 'slra-slra-b1908bf'))
%%
datafull = importdata('fake_sinft_40N_20kpt.csv');
timestamp = datafull.data(:,1);
timestamp = timestamp(500:end - 500);
timestamp = diff(timestamp);
y_force = datafull.data(:,3);
y_force = y_force(500:end - 501);
J1_vel = datafull.data(:,26);
J2_vel = datafull.data(:,27);
J3_vel = datafull.data(:,28);
J4_vel = datafull.data(:,29);
J5_vel = datafull.data(:,30);
J6_vel = datafull.data(:,31);

J1_vel = J1_vel(500:end - 500);
J2_vel = J2_vel(500:end - 500);
J3_vel = J3_vel(500:end - 500);
J4_vel = J4_vel(500:end - 500);
J5_vel = J5_vel(500:end - 500);
J6_vel = J6_vel(500:end - 500);

J1_acc = diff(J1_vel) ./ timestamp;
J2_acc = diff(J2_vel) ./ timestamp;
J3_acc = diff(J3_vel) ./ timestamp;
J4_acc = diff(J4_vel) ./ timestamp;
J5_acc = diff(J5_vel) ./ timestamp;
J6_acc = diff(J6_vel) ./ timestamp;

z_vel_vir = datafull.data(:,55);
z_vel_vir = z_vel_vir(500:end - 500);
z_acc_vir = diff(z_vel_vir) ./ timestamp;

z_vel_real = datafull.data(:,43);
z_vel_real = z_vel_real(500:end - 500);
z_acc_real = diff(z_vel_real) ./ timestamp;

% w = [J1_acc J2_acc J3_acc J4_acc J5_acc J6_acc z_acc_vir z_acc_real];
w = [z_acc_vir z_acc_real];
y = y_force;
n = 0.8;
[u_train,u_test] = train_test_split(w,n);
[y_train, y_test] = train_test_split(y,n);
w_train = [u_train y_train];
m = 2;
ell = 1;
opt_oe.exct = 1:m;
[sys, info,wh] = ident(w_train,m,ell,opt_oe);
w_test = [u_test,y_test];
%%
[M_v,wh_v] = misfit(w_test,sys);
y_predicted = wh_v(:,3);
error = y_predicted - y_test;
% figure()
% plot(y_test)
% hold on
% plot(y_predicted)
function [train,test] = train_test_split(d,n)
split = floor(size(d,1) * n);
train = d(1:split,:);
test = d(split + 1:end,:);
end
% figure(1)
% % subplot(311)
% plot(y_force)
% title('force')
% temp = linspace(0, 7* pi, 5000);
% real_force = 30 * sin(2 * temp);
