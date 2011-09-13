%function info=speckle2D(dataFile)
% This script forms a speckle pattern on the face of a simulated CCD camera by
% interfering photons that exit from the Monte Carlo simulation.

%lambda=info.lambda;%wavelength [m]
clear all;



% ------------------- Settings -------------------------------------
% Boolean values to decide if various graphs/data are formed.
createMovie =           true;
displayExitPhotons =    false;
writeSpeckleData =      true;
makeSpeckle =           true;

% Wavelength of the photons.
lambda = 532e-7;

% Distance between medium and detector.
D = 4; % [cm]

% Start time of the simulation to make the speckle pattern from.
% That is, if the Monte Carlo + K-Wave simulation (i.e. the
% acousto-optic (AO) simulation) ran for a total time 'T', 'dt' is the time
% steps that occurred in the propagation of ultrasound.  Therefore we can
% take a portion of the whole AO simulation to make a speckle pattern, or
% use the entire photon exit data.
start_time = 100; % 'dt' starting at this time.
end_time = 100;  % end time that we want to look at.

% The acceptance angle of photons leaving the medium.
acceptance_angle = 0.25;



% The figure for the speckle pattern.
if (makeSpeckle)
    speckleFigure = figure;
end





%define the camera.
CCDGrid=zeros(150,150);
%CCDdx=5.5e-2/size(CCDGrid,1);
%CCDdy=5.5e-2/size(CCDGrid,2);
CCDdx = 6e-5;
CCDdy = 6e-5;

% The aperture of the medium (window in which photons leave)
% is defined in the Monte Carlo (MC) simulation.  Therefore,
% reference the simulation settings, specifically the detector size.
% In order to calculate the distance from
% each photon exit location to pixel on the CCD camera, we must
% know the location of the pixel in 2-D space.  It is assumed
% that the CCD camera, regardless of it's size is centered at the
% same location as the aperture.  That is, it's midpoint is at
% the same center coordinates as the exit aperture in the MC simulation.
%
center.x = 0.50; % Center of the CCD (which should be the center of exit aperture).
center.y = 0.50; % Note: in cm
start_x = center.x - (size(CCDGrid,1)/2*CCDdx);
start_y = center.y - (size(CCDGrid,2)/2*CCDdy);




% ------------------------- PLANE ---------------------------
% For drawing the plane made by the CCD and visualizing all
% photons that hit it.
%
if (displayExitPhotons)
    
    rayFigure = figure;
    
    
    % Define the exit aperture, which is circular.
    t = 0:pi/360:2*pi;
    radius = 0.2;
    xy_plane = zeros(size(t)); xy_plane(:) = 1.0; % The xy-plane that the exit aperture resides on.
    color = zeros(size(t)); color(:) = 15; % Define the color for the exit aperture plane.
    hold on;
    patch(center.x + (cos(t)*radius), center.y + (sin(t)*radius), xy_plane, color);
    hold off;
    
    % Define all the vertices on the plane (i.e. CCD corners)
    x = [start_x, (center.x+(center.x-start_x)), (center.x+(center.x-start_x)), start_x];
    y = [(center.y+(center.y-start_y)), (center.y+(center.y-start_y)), start_y, start_y];
    z = [D+xy_plane(1), D+xy_plane(1), D+xy_plane(1), D+xy_plane(1)];
    color = [10, 10, 10, 10];
    patch(x,y,z,color);
    view(3);
end




% To create a movie that shows the fluctuating speckle patter (i.e.
% modulation) we loop over all exit data files, which contain information
% about the photons for a snapshot in time of the ultrasound.
if (createMovie)
    aviobj = avifile ( 'speckle-modulation', 'compression', 'None', 'fps', 5);
end

% Time the speckle generation.
tic

% Loop over all AO simulation time steps from start to end to form speckle.
for dt=start_time:1:end_time
    
    % Load the exit data from the AO simulation.
    dataFile = ['exit-aperture-', num2str(dt), '.txt'];
    data = dlmread(dataFile);
    %data = data(100:end-100,:);
    %data = data(1:1,:);
    
    display(sprintf('Time step (dt) = %i', dt));
    
    % -------------- Exit photon figure ----------------
    if (displayExitPhotons)
        
        % The normal line segment from the aperture to the CCD.
        aperture_to_CCD = [[center.x, center.y, xy_plane(1)]; ...
            [center.x, center.y, D+xy_plane(1)]];
        
        % Draw the normal connecting the aperture and CCD.
        figure(rayFigure);
        hold on;
        plot3(aperture_to_CCD(:,1), aperture_to_CCD(:,2), aperture_to_CCD(:,3), '-g');
        hold off;
        
        % for each photon
        for photon = 1:size(data, 1)
            
            % retrieve the weight of the photon.
            weight = data(photon, 1);
            
            % transmission angles of the photon when it exiting
            % the medium.
            dirx = data(photon, 2);
            diry = data(photon, 3);
            dirz = data(photon, 4);
            
            % Only plot photons that exit at a certain angle.
            if (dirz < acceptance_angle)
                continue;
            end
            
            % retrieve the stored path length in the medium.
            path_length = data(photon,5);
            
            % retrieve the exit location of the photon.
            x = data(photon,6);
            y = data(photon,7);
            z = data(photon,8);
            
            %display(sprintf('angle = %d', dirz));
            
            
            % Check for intersection of this photon with the CCD camera plane.
            %-----------------------------------------------------------------
            % Exit location of photon.
            s0 = [x, y, z];
            % The normal to the CCD.
            %
            n = [0,0,1];
            
            % Point on the CCD.
            %
            p0 = [center.x, center.y, D+z];
            
            % Create the ray to cast from the exit of the photon out into
            % space.  If this ray intersects the CCD, then 'smear' it on
            % the face of the camera as if it was a wave.
            % NOTE:
            % - To extend the ray some portion beyond the plane made by the
            %   CCD, we give the 't' value (parametric form of line) z+D.
            %
            s1 = [x + dirx*(z*D), y + diry*(z*D), z + dirz*(z*D)];
            line_segment = s1 - s0;
            ray = [s0(1), s0(2), s0(3); ...
                s1(1), s1(2), s1(3)];
            
            figure(rayFigure);
            hold on;
            plot3(ray(:,1), ray(:,2), ray(:,3));
            hold off;
            
            % From the line segment (i.e. the ray 'r') we find the 't'
            % value that gives the intersection point on the CCD.
            %
            %ti = dot(n, (p0-s0))/dot(n, s1);
            
            % we always know where the CCD plane lies. Therefore, we can
            % calculate ti.
            ti = (D)/line_segment(3);
            %if (ti >= 0 & ti <= 1)
            xi = x + line_segment(1)*(ti);
            yi = y + line_segment(2)*(ti);
            zi = z + line_segment(3)*(ti);
            
            
            intercept_to_CCD = [[xi, yi, zi]; [p0(1), p0(2), p0(3)]];
            figure(rayFigure);
            hold on;
            plot3(intercept_to_CCD(:,1),...
                intercept_to_CCD(:,2),...
                intercept_to_CCD(:,3), '-r');
            hold off;
            
            %end % if()
            
        end
    end
    
    % only grab a chunk of photons for testing.
    data = data(1:2,:);
    
     % Zero out the data for the next run.
    CCDGrid = zeros(size(CCDGrid));
    
    for k = 1:size(data,1)
        % retrieve the weight of the photon.
        weight = data(k,1);
        
        % transmission angles of the photon when it exiting
        % the medium.
        dirx = data(k, 2);
        diry = data(k, 3);
        dirz = data(k, 4);
        
        % retrieve the exit location of the photon.
        x = data(k,6);
        y = data(k,7);
        z = data(k,8);
        
        
        % retrieve the stored path length in the medium.
        path_length = data(k,5);
        
       
    
        % for each pixel in the x-axis
        for i = 1:size(CCDGrid, 1)
            x_pixel = start_x + (CCDdx * i);
            
            % for each pixel in the y-axis
            for j = 1:size(CCDGrid, 2)
                y_pixel = start_y + (CCDdy * j);
                
                % add the distance from medium to the detector pixel.
                dist_to_pixel = sqrt(D^2 + (x_pixel - x)^2 + (y_pixel - y)^2);
                
                L = path_length + dist_to_pixel;
                
                CCDGrid(i,j) = (CCDGrid(i,j) + 1/(dist_to_pixel*weight*exp(-(1i)*L*2*pi/lambda)));
                
            end
            y_pixel = start_y + CCDdy;
        end
    end
    
    figure(speckleFigure);
    CCDGrid = abs(CCDGrid).^2;
    imagesc(CCDGrid);
    %cmax = caxis;
    %caxis([cmax(2)/3 cmax(2)]);  % Scale the colormap of the image.
    %caxis auto;
    colormap hot;
    drawnow;
    if (createMovie)
        set(gcf,'Renderer', 'zbuffer');
        frame = getframe(gcf);
        aviobj = addframe(aviobj, frame);
    end
    
    % Write the CCDGrid data out to file in case it is used later.
    if (writeSpeckleData)
        speckleFile = ['speckle-data/speckle-', num2str(dt), '.txt'];
        dlmwrite(speckleFile, CCDGrid, '\t');
    end
    
    
end


toc

if (createMovie)
    aviobj = close(aviobj);
end




% -------------------------------------------------------------------------
% function Norms=getNorms(stuff)
% tic
%     CCDdx=stuff.CCDdx;
%     CCD2Surf=stuff.CCD2Surf;
%     CCDGrid=stuff.CCDGrid;
%     surfaceGrid=stuff.surfaceGrid;
%     surfacedX=stuff.surfacedX;
%     lambda=stuff.lambda;
%
% offsetx=((size(CCDGrid,1)-0.5)*CCDdx-(size(surfaceGrid,1)-0.5)*surfacedX)/1;
% offsety=((size(CCDGrid,2)-0.5)*CCDdx-(size(surfaceGrid,2)-0.5)*surfacedX)/1;
% norm1=zeros(size(surfaceGrid));
% %Norms{1:size(CCDGrid,1),1:size(CCDGrid,2)}=zeros(size(surfaceGrid));
%     for i=1:size(CCDGrid,1)
%         for j=1:size(CCDGrid,2)
%             y=[i*CCDdx j*CCDdx CCD2Surf];
%
%             for k=1:size(surfaceGrid,1);
%                 for l=1:size(surfaceGrid,2);
%                     x=[k*surfacedX-offsetx l*surfacedX-offsety 0];
%                     norm1(k,l)=norm(y-x);
%                 end
%             end
%             Norms{i,j}=exp(1i*2*pi/lambda*norm1)./norm1;
%         end
%     end
%     toc
%     disp ('geometry matices created')
%
% end